#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int build_dns_query(const char* name, uint16_t qtype, uint16_t qid, uint8_t* out, size_t* outlen) {
    if (!name || !out || !outlen) return -1;
    uint8_t* p = out;
    size_t cap = *outlen;
    if (cap < 12) return -1;
    // header
    *p++ = (qid >> 8) & 0xFF; *p++ = qid & 0xFF;
    *p++ = 0x01; *p++ = 0x00; // RD
    *p++ = 0x00; *p++ = 0x01; // QD=1
    *p++ = 0x00; *p++ = 0x00; // AN
    *p++ = 0x00; *p++ = 0x00; // NS
    *p++ = 0x00; *p++ = 0x00; // AR
    const char* s = name;
    while (*s) {
        const char* dot = strchr(s, '.');
        size_t len = dot ? (size_t)(dot - s) : strlen(s);
        if (len == 0 || len > 63) return -1;
        if ((size_t)(p - out) + 1 + len + 1 + 4 > cap) return -1;
        *p++ = (uint8_t)len;
        memcpy(p, s, len); p += len;
        if (!dot) break;
        s = dot + 1;
    }
    *p++ = 0x00;
    *p++ = (qtype >> 8) & 0xFF; *p++ = qtype & 0xFF;
    *p++ = 0x00; *p++ = 0x01; // IN
    *outlen = (size_t)(p - out);
    return 0;
}

static int parse_qname(const uint8_t* data, size_t len, size_t offset, char* name, size_t namecap, size_t* next) {
    size_t i = offset;
    size_t w = 0;
    int first = 1;
    while (1) {
        if (i >= len) return -1;
        uint8_t l = data[i++];
        if ((l & 0xC0) == 0xC0) return -1; // no compression in demo
        if (l == 0) break;
        if (l > 63 || i + l > len) return -1;
        if (!first) {
            if (w + 1 >= namecap) return -1;
            name[w++] = '.';
        }
        first = 0;
        if (w + l >= namecap) return -1;
        memcpy(name + w, data + i, l);
        w += l;
        i += l;
    }
    if (w >= namecap) return -1;
    name[w] = 0;
    *next = i;
    return 0;
}

static int build_dns_response(const uint8_t* req, size_t reqlen, uint8_t* resp, size_t* resplen) {
    if (!req || !resp || !resplen || reqlen < 12 || reqlen > 512) return -1;
    uint16_t id = (req[0] << 8) | req[1];
    uint16_t flags = (req[2] << 8) | req[3];
    uint16_t qd = (req[4] << 8) | req[5];
    uint8_t* p = resp;
    if (qd != 1) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        *p++ = (id>>8)&0xFF; *p++ = id&0xFF;
        *p++ = (rflags>>8)&0xFF; *p++ = rflags&0xFF;
        memset(p, 0, 8); p += 8;
        *resplen = (size_t)(p - resp);
        return 0;
    }
    char qname[256]; size_t next = 0;
    if (parse_qname(req, reqlen, 12, qname, sizeof(qname), &next) != 0) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        *p++ = (id>>8)&0xFF; *p++ = id&0xFF;
        *p++ = (rflags>>8)&0xFF; *p++ = rflags&0xFF;
        memset(p, 0, 8); p += 8;
        *resplen = (size_t)(p - resp);
        return 0;
    }
    if (next + 4 > reqlen) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        *p++ = (id>>8)&0xFF; *p++ = id&0xFF;
        *p++ = (rflags>>8)&0xFF; *p++ = rflags&0xFF;
        memset(p, 0, 8); p += 8;
        *resplen = (size_t)(p - resp);
        return 0;
    }
    uint16_t qtype = (req[next] << 8) | req[next+1];
    uint16_t qclass = (req[next+2] << 8) | req[next+3];
    int rcode = 0;
    uint32_t ip = 0;
    if (qclass != 1) {
        rcode = 4;
    } else if (qtype == 1) {
        char lower[256]; size_t i;
        for (i=0; qname[i] && i<sizeof(lower)-1; ++i) lower[i] = (char)tolower((unsigned char)qname[i]);
        lower[i] = 0;
        if (strcmp(lower, "example.com") == 0) inet_pton(AF_INET, "93.184.216.34", &ip);
        else if (strcmp(lower, "localhost") == 0) inet_pton(AF_INET, "127.0.0.1", &ip);
        else if (strcmp(lower, "test.local") == 0) inet_pton(AF_INET, "192.168.1.2", &ip);
        else rcode = 3;
    } else {
        rcode = 4;
    }
    uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | (rcode & 0xF);
    *p++ = (id>>8)&0xFF; *p++ = id&0xFF;
    *p++ = (rflags>>8)&0xFF; *p++ = rflags&0xFF;
    *p++ = 0x00; *p++ = 0x01;
    if (rcode == 0 && qtype == 1 && ip != 0) { *p++ = 0x00; *p++ = 0x01; }
    else { *p++ = 0x00; *p++ = 0x00; }
    *p++ = 0x00; *p++ = 0x00;
    *p++ = 0x00; *p++ = 0x00;
    size_t qlen = (next + 4) - 12;
    if ((size_t)(p - resp) + qlen > *resplen) return -1;
    memcpy(p, req + 12, qlen); p += qlen;
    if (rcode == 0 && qtype == 1 && ip != 0) {
        // NAME pointer
        *p++ = 0xC0; *p++ = 0x0C;
        // TYPE A
        *p++ = 0x00; *p++ = 0x01;
        // CLASS IN
        *p++ = 0x00; *p++ = 0x01;
        // TTL 60
        *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x3C;
        // RDLENGTH 4
        *p++ = 0x00; *p++ = 0x04;
        // RDATA
        uint8_t* ipb = (uint8_t*)&ip;
        *p++ = ipb[0]; *p++ = ipb[1]; *p++ = ipb[2]; *p++ = ipb[3];
    }
    *resplen = (size_t)(p - resp);
    return 0;
}

static void run_server(const char* bind_ip, uint16_t port, int max_requests) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_port = htons(port);
    inet_pton(AF_INET, bind_ip, &addr.sin_addr);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    int handled = 0;
    uint8_t buf[512];
    while (handled < max_requests) {
        struct sockaddr_in src; socklen_t sl = sizeof(src);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &sl);
        if (n <= 0) continue;
        uint8_t resp[512]; size_t resplen = sizeof(resp);
        if (build_dns_response(buf, (size_t)n, resp, &resplen) == 0) {
            sendto(fd, resp, resplen, 0, (struct sockaddr*)&src, sl);
        }
        handled++;
    }
    close(fd);
}

static int parse_a_from_response(const uint8_t* resp, size_t len, char* outip, size_t outlen) {
    if (len < 12) return -1;
    uint16_t an = (resp[6] << 8) | resp[7];
    if (an < 1) return -1;
    size_t i = 12;
    while (i < len && resp[i] != 0) {
        uint8_t l = resp[i];
        if (l & 0xC0) { if (i+1 >= len) return -1; i += 2; break; }
        i += 1 + l;
    }
    if (i >= len) return -1;
    if (resp[i] == 0) i++;
    i += 4;
    if (i >= len) return -1;
    if (resp[i] & 0xC0) i += 2;
    else {
        while (i < len && resp[i] != 0) { uint8_t l = resp[i]; i += 1 + l; }
        i++;
    }
    if (i + 10 > len) return -1;
    uint16_t type = (resp[i] << 8) | resp[i+1]; i += 2;
    i += 2; // class
    i += 4; // ttl
    uint16_t rdlen = (resp[i] << 8) | resp[i+1]; i += 2;
    if (type != 1 || rdlen != 4 || i + 4 > len) return -1;
    struct in_addr a; memcpy(&a, resp + i, 4);
    const char* s = inet_ntop(AF_INET, &a, outip, (socklen_t)outlen);
    return s ? 0 : -1;
}

int main() {
    const char* ip = "127.0.0.1";
    uint16_t port = 53538;
    pid_t pid = fork();
    if (pid == 0) {
        run_server(ip, port, 5);
        return 0;
    }
    usleep(200000);
    struct { const char* name; uint16_t type; } tests[5] = {
        {"example.com",1},
        {"localhost",1},
        {"test.local",1},
        {"unknown.example",1},
        {"example.com",28}
    };
    for (int i=0;i<5;i++) {
        uint8_t q[512]; size_t qlen = sizeof(q);
        if (build_dns_query(tests[i].name, tests[i].type, (uint16_t)(0x4444+i), q, &qlen) != 0) {
            printf("C Test %d build fail\n", i+1);
            continue;
        }
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        struct timeval tv; tv.tv_sec = 2; tv.tv_usec = 0;
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        struct sockaddr_in dst; memset(&dst,0,sizeof(dst));
        dst.sin_family = AF_INET; dst.sin_port = htons(port);
        inet_pton(AF_INET, ip, &dst.sin_addr);
        sendto(fd, q, qlen, 0, (struct sockaddr*)&dst, sizeof(dst));
        uint8_t buf[512]; struct sockaddr_in src; socklen_t sl = sizeof(src);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&src, &sl);
        if (n > 0) {
            char out[64];
            if (parse_a_from_response(buf, (size_t)n, out, sizeof(out)) == 0) {
                printf("C Test %d %s type %u -> %s\n", i+1, tests[i].name, tests[i].type, out);
            } else {
                printf("C Test %d %s type %u -> NOANSWER\n", i+1, tests[i].name, tests[i].type);
            }
        } else {
            printf("C Test %d timeout\n", i+1);
        }
        close(fd);
    }
    return 0;
}