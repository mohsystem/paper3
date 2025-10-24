#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define DNS_MAX_UDP_SIZE 512
#define DNS_CLASS_IN 1
#define DNS_TYPE_A 1
#define DNS_TYPE_AAAA 28
#define DNS_TYPE_TXT 16

static uint16_t read_u16(const uint8_t* buf, size_t len, size_t off, bool* ok) {
    if (off + 2 > len) {
        *ok = false;
        return 0;
    }
    *ok = true;
    return (uint16_t)((buf[off] << 8) | buf[off + 1]);
}
static uint32_t read_u32(const uint8_t* buf, size_t len, size_t off, bool* ok) {
    if (off + 4 > len) {
        *ok = false;
        return 0;
    }
    *ok = true;
    return ((uint32_t)buf[off] << 24) | ((uint32_t)buf[off + 1] << 16) |
           ((uint32_t)buf[off + 2] << 8) | (uint32_t)buf[off + 3];
}
static void write_u16(uint8_t* out, size_t* off, size_t cap, uint16_t v, bool* ok) {
    if (*off + 2 > cap) {
        *ok = false;
        return;
    }
    out[(*off)++] = (uint8_t)((v >> 8) & 0xFF);
    out[(*off)++] = (uint8_t)(v & 0xFF);
}
static void write_u32(uint8_t* out, size_t* off, size_t cap, uint32_t v, bool* ok) {
    if (*off + 4 > cap) {
        *ok = false;
        return;
    }
    out[(*off)++] = (uint8_t)((v >> 24) & 0xFF);
    out[(*off)++] = (uint8_t)((v >> 16) & 0xFF);
    out[(*off)++] = (uint8_t)((v >> 8) & 0xFF);
    out[(*off)++] = (uint8_t)(v & 0xFF);
}

static bool parse_domain_name(const uint8_t* buf, size_t len, size_t* offset, char* out, size_t outlen) {
    size_t off = *offset;
    size_t jumps = 0;
    const size_t MAX_JUMPS = 20;
    bool jumped = false;
    size_t outpos = 0;

    while (1) {
        if (off >= len) return false;
        uint8_t labellen = buf[off];
        if ((labellen & 0xC0) == 0xC0) {
            if (off + 1 >= len) return false;
            uint16_t ptr = (uint16_t)(((labellen & 0x3F) << 8) | buf[off + 1]);
            if (ptr >= len) return false;
            if (++jumps > MAX_JUMPS) return false;
            off = ptr;
            jumped = true;
            continue;
        } else if (labellen == 0) {
            if (!jumped) *offset = off + 1;
            break;
        } else {
            if (labellen > 63) return false;
            if (off + 1 + labellen > len) return false;
            if (outpos != 0) {
                if (outpos + 1 >= outlen) return false;
                out[outpos++] = '.';
            }
            for (size_t i = 0; i < labellen; ++i) {
                char c = (char)buf[off + 1 + i];
                if (outpos + 1 >= outlen) return false;
                if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
                out[outpos++] = c;
            }
            off += 1 + labellen;
            if (!jumped) *offset = off;
        }
    }
    if (outpos >= outlen) return false;
    out[outpos] = '\0';
    return true;
}

static bool write_domain_name(uint8_t* out, size_t* off, size_t cap, const char* name) {
    size_t nlen = strlen(name);
    if (nlen == 0) {
        if (*off + 1 > cap) return false;
        out[(*off)++] = 0;
        return true;
    }
    if (nlen > 255) return false;
    size_t start = 0;
    for (size_t i = 0; i <= nlen; ++i) {
        if (name[i] == '.' || name[i] == '\0') {
            size_t labellen = i - start;
            if (labellen == 0 || labellen > 63) return false;
            if (*off + 1 + labellen > cap) return false;
            out[(*off)++] = (uint8_t)labellen;
            for (size_t j = 0; j < labellen; ++j) out[(*off)++] = (uint8_t)name[start + j];
            start = i + 1;
        }
    }
    if (*off + 1 > cap) return false;
    out[(*off)++] = 0;
    return true;
}

typedef struct {
    uint16_t type;
    uint16_t cls;
    uint32_t ttl;
    uint8_t rdata[256];
    uint16_t rdlen;
} DNSAnswer;

static bool resolve_locally(const char* qname, uint16_t qtype, DNSAnswer* out, size_t* out_count) {
    *out_count = 0;
    if (strcmp(qname, "localhost") == 0) {
        if (qtype == DNS_TYPE_A || qtype == 255) {
            out[*out_count].type = DNS_TYPE_A;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 60;
            out[*out_count].rdata[0] = 127;
            out[*out_count].rdata[1] = 0;
            out[*out_count].rdata[2] = 0;
            out[*out_count].rdata[3] = 1;
            out[*out_count].rdlen = 4;
            (*out_count)++;
        }
        if (qtype == DNS_TYPE_AAAA || qtype == 255) {
            out[*out_count].type = DNS_TYPE_AAAA;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 60;
            memset(out[*out_count].rdata, 0, 16);
            out[*out_count].rdata[15] = 1;
            out[*out_count].rdlen = 16;
            (*out_count)++;
        }
        return (*out_count) > 0;
    } else if (strcmp(qname, "example.com") == 0) {
        if (qtype == DNS_TYPE_A || qtype == 255) {
            out[*out_count].type = DNS_TYPE_A;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 300;
            out[*out_count].rdata[0] = 93;
            out[*out_count].rdata[1] = 184;
            out[*out_count].rdata[2] = 216;
            out[*out_count].rdata[3] = 34;
            out[*out_count].rdlen = 4;
            (*out_count)++;
        }
        return (*out_count) > 0;
    } else if (strcmp(qname, "test.local") == 0) {
        if (qtype == DNS_TYPE_A || qtype == 255) {
            out[*out_count].type = DNS_TYPE_A;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 120;
            out[*out_count].rdata[0] = 192;
            out[*out_count].rdata[1] = 0;
            out[*out_count].rdata[2] = 2;
            out[*out_count].rdata[3] = 1;
            out[*out_count].rdlen = 4;
            (*out_count)++;
        }
        return (*out_count) > 0;
    } else if (strcmp(qname, "ipv6.local") == 0) {
        if (qtype == DNS_TYPE_AAAA || qtype == 255) {
            out[*out_count].type = DNS_TYPE_AAAA;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 120;
            memset(out[*out_count].rdata, 0, 16);
            out[*out_count].rdata[15] = 1;
            out[*out_count].rdlen = 16;
            (*out_count)++;
        }
        return (*out_count) > 0;
    } else if (strcmp(qname, "txt.local") == 0) {
        if (qtype == DNS_TYPE_TXT || qtype == 255) {
            const char* s = "v=demo";
            size_t slen = strlen(s);
            if (slen > 255) return false;
            out[*out_count].type = DNS_TYPE_TXT;
            out[*out_count].cls = DNS_CLASS_IN;
            out[*out_count].ttl = 120;
            out[*out_count].rdata[0] = (uint8_t)slen;
            memcpy(&out[*out_count].rdata[1], s, slen);
            out[*out_count].rdlen = (uint16_t)(1 + slen);
            (*out_count)++;
            return true;
        }
        return false;
    }
    return false;
}

static size_t build_error_response(const uint8_t* req, size_t req_len, uint8_t* resp, size_t cap, uint16_t rcode) {
    if (cap < 12 || req_len < 12) return 0;
    size_t off = 0;
    // ID
    resp[off++] = req[0];
    resp[off++] = req[1];
    // Flags
    uint16_t flags = (uint16_t)((req[2] << 8) | req[3]);
    flags |= 0x8000; // QR
    flags &= (uint16_t)~0x0200; // TC clear
    flags &= (uint16_t)~0x0010; // RA clear
    flags &= (uint16_t)~0x000F; // clear RCODE
    flags |= (rcode & 0x000F);
    bool ok = true;
    write_u16(resp, &off, cap, flags, &ok);
    // QD
    resp[off++] = req[4];
    resp[off++] = req[5];
    // AN, NS, AR = 0
    write_u16(resp, &off, cap, 0, &ok);
    write_u16(resp, &off, cap, 0, &ok);
    write_u16(resp, &off, cap, 0, &ok);
    // Echo one question if present
    size_t qoff = 12;
    char tmp[256];
    size_t qoff_copy = qoff;
    if (parse_domain_name(req, req_len, &qoff_copy, tmp, sizeof(tmp))) {
        size_t qlen = qoff_copy - qoff;
        if (qoff_copy + 4 <= req_len && off + qlen + 4 <= cap) {
            memcpy(&resp[off], &((uint8_t*)req)[qoff], qlen);
            off += qlen;
            memcpy(&resp[off], &((uint8_t*)req)[qoff_copy], 4);
            off += 4;
        }
    }
    return off;
}

static size_t handle_dns_request(const uint8_t* req, size_t req_len, uint8_t* resp, size_t cap) {
    if (req_len < 12 || req_len > DNS_MAX_UDP_SIZE || cap < 12) {
        return build_error_response(req, req_len, resp, cap, 1);
    }
    bool ok = true;
    uint16_t id = read_u16(req, req_len, 0, &ok);
    if (!ok) return build_error_response(req, req_len, resp, cap, 1);
    uint16_t flags = read_u16(req, req_len, 2, &ok);
    if (!ok) return build_error_response(req, req_len, resp, cap, 1);
    uint16_t qdcount = read_u16(req, req_len, 4, &ok);
    if (!ok || qdcount != 1) return build_error_response(req, req_len, resp, cap, 1);

    size_t off = 12;
    char qname[256];
    if (!parse_domain_name(req, req_len, &off, qname, sizeof(qname))) {
        return build_error_response(req, req_len, resp, cap, 1);
    }
    uint16_t qtype = read_u16(req, req_len, off, &ok); if (!ok) return build_error_response(req, req_len, resp, cap, 1);
    off += 2;
    uint16_t qclass = read_u16(req, req_len, off, &ok); if (!ok) return build_error_response(req, req_len, resp, cap, 1);
    off += 2;
    if (qclass != DNS_CLASS_IN) return build_error_response(req, req_len, resp, cap, 4);

    DNSAnswer answers[4];
    size_t ancount = 0;
    bool found = resolve_locally(qname, qtype, answers, &ancount);

    // Build response
    size_t roff = 0;
    write_u16(resp, &roff, cap, id, &ok);
    uint16_t rflags = 0x8000; // QR
    rflags |= 0x0400;         // AA
    if (flags & 0x0100) rflags |= 0x0100; // RD
    uint16_t rcode = found ? 0 : 3;
    rflags |= (rcode & 0x000F);
    write_u16(resp, &roff, cap, rflags, &ok);
    write_u16(resp, &roff, cap, 1, &ok);
    write_u16(resp, &roff, cap, found ? (uint16_t)ancount : 0, &ok);
    write_u16(resp, &roff, cap, 0, &ok);
    write_u16(resp, &roff, cap, 0, &ok);
    if (!write_domain_name(resp, &roff, cap, qname)) return build_error_response(req, req_len, resp, cap, 1);
    write_u16(resp, &roff, cap, qtype, &ok);
    write_u16(resp, &roff, cap, DNS_CLASS_IN, &ok);
    if (!ok) return build_error_response(req, req_len, resp, cap, 1);

    if (found) {
        for (size_t i = 0; i < ancount; ++i) {
            if (!write_domain_name(resp, &roff, cap, qname)) return build_error_response(req, req_len, resp, cap, 1);
            write_u16(resp, &roff, cap, answers[i].type, &ok);
            write_u16(resp, &roff, cap, answers[i].cls, &ok);
            write_u32(resp, &roff, cap, answers[i].ttl, &ok);
            write_u16(resp, &roff, cap, answers[i].rdlen, &ok);
            if (!ok || roff + answers[i].rdlen > cap) return build_error_response(req, req_len, resp, cap, 1);
            memcpy(&resp[roff], answers[i].rdata, answers[i].rdlen);
            roff += answers[i].rdlen;
            if (roff > DNS_MAX_UDP_SIZE) break;
        }
    }
    return roff;
}

static size_t make_dns_query(uint8_t* out, size_t cap, uint16_t id, const char* qname, uint16_t qtype) {
    size_t off = 0;
    bool ok = true;
    write_u16(out, &off, cap, id, &ok);
    write_u16(out, &off, cap, 0x0100, &ok); // RD
    write_u16(out, &off, cap, 1, &ok);      // QD
    write_u16(out, &off, cap, 0, &ok);
    write_u16(out, &off, cap, 0, &ok);
    write_u16(out, &off, cap, 0, &ok);
    if (!ok) return 0;
    if (!write_domain_name(out, &off, cap, qname)) return 0;
    write_u16(out, &off, cap, qtype, &ok);
    write_u16(out, &off, cap, DNS_CLASS_IN, &ok);
    if (!ok) return 0;
    return off;
}

static bool parse_resp_header(const uint8_t* r, size_t len, uint16_t* id, uint16_t* rcode, uint16_t* ancount) {
    bool ok = true;
    if (len < 12) return false;
    *id = read_u16(r, len, 0, &ok); if (!ok) return false;
    uint16_t flags = read_u16(r, len, 2, &ok); if (!ok) return false;
    *ancount = read_u16(r, len, 6, &ok); if (!ok) return false;
    *rcode = (uint16_t)(flags & 0x000F);
    return true;
}

typedef struct {
    uint16_t port;
} server_args_t;

static void* server_thread(void* arg) {
    server_args_t* sarg = (server_args_t*)arg;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        sarg->port = 0;
        return NULL;
    }
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0); // let OS choose
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        sarg->port = 0;
        return NULL;
    }
    socklen_t alen = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*)&addr, &alen) == 0) {
        sarg->port = ntohs(addr.sin_port);
    } else {
        sarg->port = 0;
    }
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    for (int i = 0; i < 2; ++i) {
        uint8_t buf[DNS_MAX_UDP_SIZE];
        struct sockaddr_in cli;
        socklen_t clen = sizeof(cli);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cli, &clen);
        if (n < 0) break;
        uint8_t resp[DNS_MAX_UDP_SIZE];
        size_t rlen = handle_dns_request(buf, (size_t)n, resp, sizeof(resp));
        if (rlen > 0) {
            sendto(fd, resp, rlen, 0, (struct sockaddr*)&cli, clen);
        }
    }
    close(fd);
    return NULL;
}

int main(void) {
    // Test 1: write and parse domain name
    {
        uint8_t tmp[256];
        size_t off = 0;
        bool ok = write_domain_name(tmp, &off, sizeof(tmp), "Example.COM", 0);
        if (!ok) {
            printf("Test1: write failed\n");
        } else {
            size_t poff = 0;
            char name[256];
            if (!parse_domain_name(tmp, off, &poff, name, sizeof(name))) {
                printf("Test1: parse failed\n");
            } else {
                printf("Test1: %s (len=%zu)\n", name, off);
            }
        }
    }

    // Test 2: local resolver localhost A
    {
        DNSAnswer ans[4];
        size_t cnt = 0;
        bool ok = resolve_locally("localhost", DNS_TYPE_A, ans, &cnt);
        printf("Test2: localhost A resolved=%s answers=%zu\n", ok ? "yes" : "no", cnt);
    }

    // Test 3: handle crafted query example.com A
    {
        uint8_t q[DNS_MAX_UDP_SIZE];
        size_t qlen = make_dns_query(q, sizeof(q), 0x1234, "example.com", DNS_TYPE_A);
        uint8_t r[DNS_MAX_UDP_SIZE];
        size_t rlen = handle_dns_request(q, qlen, r, sizeof(r));
        uint16_t id = 0, rcode = 0, an = 0;
        bool ok = parse_resp_header(r, rlen, &id, &rcode, &an);
        printf("Test3: ok=%s id=0x%04x rcode=%u ancount=%u\n", ok ? "yes" : "no", id, rcode, an);
    }

    // Start server thread (bind to 127.0.0.1:ephemeral) and send two queries
    server_args_t sarg;
    sarg.port = 0;
    pthread_t th;
    if (pthread_create(&th, NULL, server_thread, &sarg) != 0) {
        printf("Server thread creation failed\n");
        return 0;
    }
    // Wait a moment for server to bind (simple sleep to avoid complexity)
    usleep(200000);

    uint16_t srv_port = sarg.port;
    if (srv_port == 0) {
        printf("Server failed to bind\n");
    } else {
        // Test 4: localhost A query to server
        int cfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (cfd >= 0) {
            struct sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_port = htons(srv_port);
            inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);
            uint8_t q[DNS_MAX_UDP_SIZE];
            size_t qlen = make_dns_query(q, sizeof(q), 0xA001, "localhost", DNS_TYPE_A);
            sendto(cfd, q, qlen, 0, (struct sockaddr*)&sa, sizeof(sa));
            uint8_t buf[DNS_MAX_UDP_SIZE];
            struct sockaddr_in from;
            socklen_t flen = sizeof(from);
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            ssize_t n = recvfrom(cfd, buf, sizeof(buf), 0, (struct sockaddr*)&from, &flen);
            if (n > 0) {
                uint16_t id = 0, rcode = 0, an = 0;
                bool ok = parse_resp_header(buf, (size_t)n, &id, &rcode, &an);
                printf("Test4: recv=%zd ok=%s id=0x%04x rcode=%u ancount=%u\n", n, ok ? "yes" : "no", id, rcode, an);
            } else {
                printf("Test4: no response\n");
            }
            close(cfd);
        } else {
            printf("Test4: client socket failed\n");
        }

        // Test 5: ipv6.local AAAA query to server
        int cfd2 = socket(AF_INET, SOCK_DGRAM, 0);
        if (cfd2 >= 0) {
            struct sockaddr_in sa;
            memset(&sa, 0, sizeof(sa));
            sa.sin_family = AF_INET;
            sa.sin_port = htons(srv_port);
            inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);
            uint8_t q[DNS_MAX_UDP_SIZE];
            size_t qlen = make_dns_query(q, sizeof(q), 0xA002, "ipv6.local", DNS_TYPE_AAAA);
            sendto(cfd2, q, qlen, 0, (struct sockaddr*)&sa, sizeof(sa));
            uint8_t buf[DNS_MAX_UDP_SIZE];
            struct sockaddr_in from;
            socklen_t flen = sizeof(from);
            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            setsockopt(cfd2, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            ssize_t n = recvfrom(cfd2, buf, sizeof(buf), 0, (struct sockaddr*)&from, &flen);
            if (n > 0) {
                uint16_t id = 0, rcode = 0, an = 0;
                bool ok = parse_resp_header(buf, (size_t)n, &id, &rcode, &an);
                printf("Test5: recv=%zd ok=%s id=0x%04x rcode=%u ancount=%u\n", n, ok ? "yes" : "no", id, rcode, an);
            } else {
                printf("Test5: no response\n");
            }
            close(cfd2);
        } else {
            printf("Test5: client socket failed\n");
        }
    }

    pthread_join(th, NULL);
    return 0;
}