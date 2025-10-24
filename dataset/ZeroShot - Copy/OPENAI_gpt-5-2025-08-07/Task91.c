#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define TYPE_A 1
#define TYPE_AAAA 28

typedef struct {
    uint16_t id;
    uint16_t flags;
    char qname[256];
    uint16_t qtype;
    uint16_t qclass;
    size_t question_end;
} DnsQuery;

static int encode_name(const char* name, uint8_t* out, size_t outlen) {
    size_t pos = 0;
    if (!name || !*name) {
        if (outlen < 1) return -1;
        out[0] = 0;
        return 1;
    }
    const char* p = name;
    while (*p) {
        const char* dot = strchr(p, '.');
        size_t len = dot ? (size_t)(dot - p) : strlen(p);
        if (len == 0 || len > 63) return -1;
        if (pos + 1 + len + 1 > outlen) return -1;
        out[pos++] = (uint8_t)len;
        memcpy(out + pos, p, len);
        pos += len;
        if (!dot) break;
        p = dot + 1;
    }
    if (pos + 1 > outlen) return -1;
    out[pos++] = 0;
    return (int)pos;
}

static int decode_name(const uint8_t* data, size_t len, size_t* offset, char* out, size_t outlen) {
    size_t o = *offset;
    size_t w = 0;
    bool jumped = false;
    size_t jump_end = 0;
    int depth = 0;

    while (1) {
        if (o >= len) return -1;
        uint8_t l = data[o];
        if ((l & 0xC0) == 0xC0) {
            if (o + 1 >= len) return -1;
            uint16_t ptr = ((l & 0x3F) << 8) | data[o + 1];
            if (++depth > 10) return -1;
            if (!jumped) {
                jump_end = o + 2;
                jumped = true;
            }
            o = ptr;
            continue;
        } else if (l == 0) {
            o++;
            break;
        } else {
            o++;
            if (o + l > len) return -1;
            if (w != 0) {
                if (w + 1 >= outlen) return -1;
                out[w++] = '.';
            }
            if (w + l >= outlen) return -1;
            memcpy(out + w, data + o, l);
            w += l;
            o += l;
        }
    }
    if (w >= outlen) return -1;
    out[w] = '\0';
    *offset = jumped ? jump_end : o;
    return 0;
}

static int parse_query(const uint8_t* data, size_t len, DnsQuery* q) {
    if (len < 12) return -1;
    q->id = (data[0] << 8) | data[1];
    q->flags = (data[2] << 8) | data[3];
    uint16_t qd = (data[4] << 8) | data[5];
    if (qd < 1) return -1;
    size_t off = 12;
    if (decode_name(data, len, &off, q->qname, sizeof(q->qname)) != 0) return -1;
    if (off + 4 > len) return -1;
    q->qtype = (data[off] << 8) | data[off + 1];
    q->qclass = (data[off + 2] << 8) | data[off + 3];
    q->question_end = off + 4;
    return 0;
}

static int resolve(const char* name, uint16_t qtype, uint8_t addrs[][16], size_t* count, size_t max) {
    *count = 0;
    if (strcasecmp(name, "localhost") == 0) {
        if (qtype == TYPE_A && *count < max) {
            inet_pton(AF_INET, "127.0.0.1", addrs[*count]);
            (*count)++;
        }
        if (qtype == TYPE_AAAA && *count < max) {
            inet_pton(AF_INET6, "::1", addrs[*count]);
            (*count)++;
        }
        return 0;
    }
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = (qtype == TYPE_A) ? AF_INET : (qtype == TYPE_AAAA ? AF_INET6 : AF_UNSPEC);
    if (getaddrinfo(name, NULL, &hints, &res) != 0 || !res) return -1;
    for (struct addrinfo* p = res; p && *count < max; p = p->ai_next) {
        if (qtype == TYPE_A && p->ai_family == AF_INET) {
            memcpy(addrs[*count], &((struct sockaddr_in*)p->ai_addr)->sin_addr, 4);
            (*count)++;
        } else if (qtype == TYPE_AAAA && p->ai_family == AF_INET6) {
            memcpy(addrs[*count], &((struct sockaddr_in6*)p->ai_addr)->sin6_addr, 16);
            (*count)++;
        }
    }
    freeaddrinfo(res);
    return 0;
}

static size_t build_response(const uint8_t* req, size_t reqlen, uint8_t* resp, size_t resplen) {
    DnsQuery q;
    int ok = parse_query(req, reqlen, &q);
    if (ok != 0) {
        if (resplen < 12) return 0;
        resp[0] = req[0]; resp[1] = req[1];
        resp[2] = 0x80; resp[3] = 0x02; // SERVFAIL
        memset(resp + 4, 0, 8);
        return 12;
    }
    bool supported = (q.qclass == 1) && (q.qtype == TYPE_A || q.qtype == TYPE_AAAA);
    uint8_t addrs[8][16];
    size_t acount = 0;
    if (supported) resolve(q.qname, q.qtype, addrs, &acount, 8);

    int rcode = supported ? (acount ? 0 : 3) : 4;

    if (resplen < 12) return 0;
    uint16_t flags = 0x8000 | (q.flags & 0x7800) | (q.flags & 0x0100) | (rcode & 0xF);
    resp[0] = (q.id >> 8) & 0xFF; resp[1] = q.id & 0xFF;
    resp[2] = (flags >> 8) & 0xFF; resp[3] = flags & 0xFF;
    resp[4] = 0x00; resp[5] = 0x01;
    resp[6] = 0x00; resp[7] = (rcode == 0) ? (uint8_t)acount : 0;
    resp[8] = 0x00; resp[9] = 0x00;
    resp[10] = 0x00; resp[11] = 0x00;

    size_t pos = 12;
    size_t qlen = q.question_end - 12;
    if (pos + qlen > resplen) return 0;
    memcpy(resp + pos, req + 12, qlen);
    pos += qlen;

    if (rcode == 0) {
        for (size_t i = 0; i < acount; i++) {
            if (pos + 12 + ((q.qtype == TYPE_A) ? 4 : 16) > resplen) break;
            // name pointer to 0xC00C
            resp[pos++] = 0xC0; resp[pos++] = 0x0C;
            resp[pos++] = (q.qtype >> 8) & 0xFF; resp[pos++] = q.qtype & 0xFF;
            resp[pos++] = 0x00; resp[pos++] = 0x01; // IN
            resp[pos++] = 0x00; resp[pos++] = 0x00; resp[pos++] = 0x00; resp[pos++] = 0x3C; // TTL 60
            uint16_t rdlen = (q.qtype == TYPE_A) ? 4 : 16;
            resp[pos++] = (rdlen >> 8) & 0xFF; resp[pos++] = rdlen & 0xFF;
            memcpy(resp + pos, addrs[i], rdlen);
            pos += rdlen;
        }
    }
    if (pos > resplen) pos = resplen;
    return pos;
}

static int build_query(uint16_t id, const char* name, uint16_t qtype, uint8_t* out, size_t outlen, size_t* outsz) {
    if (outlen < 12) return -1;
    out[0] = (id >> 8) & 0xFF; out[1] = id & 0xFF;
    out[2] = 0x01; out[3] = 0x00; // RD=1
    out[4] = 0x00; out[5] = 0x01;
    out[6] = out[7] = out[8] = out[9] = out[10] = out[11] = 0;
    size_t pos = 12;
    int nlen = encode_name(name, out + pos, outlen - pos);
    if (nlen < 0) return -1;
    pos += (size_t)nlen;
    if (pos + 4 > outlen) return -1;
    out[pos++] = (qtype >> 8) & 0xFF; out[pos++] = qtype & 0xFF;
    out[pos++] = 0x00; out[pos++] = 0x01;
    *outsz = pos;
    return 0;
}

typedef struct {
    char ip[32];
    int port;
    int seconds;
} server_args_t;

void* server_thread(void* arg) {
    server_args_t* a = (server_args_t*)arg;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return NULL;
    struct timeval tv; tv.tv_sec = 0; tv.tv_usec = 500000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(a->port);
    inet_pton(AF_INET, a->ip, &addr.sin_addr);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return NULL;
    }
    time_t end = time(NULL) + a->seconds;
    uint8_t buf[2048], resp[1500];
    while (time(NULL) < end) {
        struct sockaddr_in cli; socklen_t clen = sizeof(cli);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cli, &clen);
        if (n < 0) continue;
        size_t rsz = build_response(buf, (size_t)n, resp, sizeof(resp));
        if (rsz > 0) {
            sendto(fd, resp, rsz, 0, (struct sockaddr*)&cli, clen);
        }
    }
    close(fd);
    return NULL;
}

static const char* rcode_name(int r) {
    switch (r) {
        case 0: return "NOERROR";
        case 2: return "SERVFAIL";
        case 3: return "NXDOMAIN";
        case 4: return "NOTIMP";
        default: return "RCODE";
    }
}

int main(void) {
    server_args_t args;
    strcpy(args.ip, "127.0.0.1");
    args.port = 8056;
    args.seconds = 5;
    pthread_t th;
    pthread_create(&th, NULL, server_thread, &args);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval tv; tv.tv_sec = 1; tv.tv_usec = 500000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in srv; memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(args.port);
    inet_pton(AF_INET, args.ip, &srv.sin_addr);

    const char* names[5] = {"localhost", "localhost", "example.com", "nonexistent.invalid", "example.com"};
    int types[5] = {TYPE_A, TYPE_AAAA, TYPE_A, TYPE_A, 16};

    for (int i = 0; i < 5; i++) {
        uint8_t q[512]; size_t qsz = 0;
        build_query(0x4000 + i, names[i], (uint16_t)types[i], q, sizeof(q), &qsz);
        sendto(fd, q, qsz, 0, (struct sockaddr*)&srv, sizeof(srv));
        uint8_t buf[2048];
        struct sockaddr_in from; socklen_t flen = sizeof(from);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&from, &flen);
        if (n >= 12) {
            int id = (buf[0] << 8) | buf[1];
            int flags = (buf[2] << 8) | buf[3];
            int an = (buf[6] << 8) | buf[7];
            int rcode = flags & 0xF;
            printf("Test %d: id=%d answers=%d rcode=%s\n", i + 1, id, an, rcode_name(rcode));
        } else {
            printf("Test %d: timeout/empty\n", i + 1);
        }
    }

    close(fd);
    pthread_join(th, NULL);
    return 0;
}