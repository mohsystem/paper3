#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    uint16_t id;
    uint16_t flags;
    char qname[256];
    uint16_t qtype;
    uint16_t qclass;
    size_t questionEnd;
} ParsedQuery;

static uint16_t rd16(const uint8_t* p){ return (uint16_t)((p[0]<<8)|p[1]); }
static void wr16(uint8_t* b, size_t* o, uint16_t v){ b[(*o)++]=(uint8_t)(v>>8); b[(*o)++]=(uint8_t)(v&0xFF); }
static void wr32(uint8_t* b, size_t* o, uint32_t v){ b[(*o)++]=(v>>24)&0xFF; b[(*o)++]=(v>>16)&0xFF; b[(*o)++]=(v>>8)&0xFF; b[(*o)++]=v&0xFF; }

int parseDnsQuery(const uint8_t* data, size_t len, ParsedQuery* pq){
    if (len < 12) return -1;
    pq->id = rd16(&data[0]);
    pq->flags = rd16(&data[2]);
    uint16_t qd = rd16(&data[4]);
    if (qd < 1) return -2;
    size_t idx = 12;
    size_t name_pos = 0;
    pq->qname[0] = '\0';
    while (idx < len) {
        uint8_t ln = data[idx++];
        if (ln == 0) break;
        if (idx + ln > len) return -3;
        if (name_pos && name_pos < sizeof(pq->qname)-1) pq->qname[name_pos++]='.';
        for (uint8_t i=0;i<ln && name_pos < sizeof(pq->qname)-1;i++) {
            pq->qname[name_pos++] = (char)data[idx+i];
        }
        idx += ln;
    }
    pq->qname[name_pos] = '\0';
    if (idx + 4 > len) return -4;
    pq->qtype = rd16(&data[idx]);
    pq->qclass = rd16(&data[idx+2]);
    pq->questionEnd = idx + 4;
    return 0;
}

static int ip_to_bytes(const char* ip, uint8_t out[4]){
    int b0,b1,b2,b3;
    if (sscanf(ip, "%d.%d.%d.%d", &b0,&b1,&b2,&b3) != 4) return -1;
    out[0]= (uint8_t)(b0&0xFF); out[1]=(uint8_t)(b1&0xFF); out[2]=(uint8_t)(b2&0xFF); out[3]=(uint8_t)(b3&0xFF);
    return 0;
}

typedef struct {
    const char* key;
    const char* val;
} kv_t;

const char* hosts_lookup(const kv_t* hosts, size_t n, const char* name){
    static char lower[256];
    size_t i;
    for (i=0; name[i] && i<sizeof(lower)-1; ++i) lower[i] = (char)tolower((unsigned char)name[i]);
    lower[i] = '\0';
    for (size_t j=0;j<n;j++){
        if (strcasecmp(hosts[j].key, lower)==0) return hosts[j].val;
    }
    return NULL;
}

size_t buildDnsResponse(const uint8_t* request, size_t rlen, const kv_t* hosts, size_t hlen, uint8_t* out, size_t outcap){
    ParsedQuery pq;
    if (parseDnsQuery(request, rlen, &pq) != 0) return 0;
    int rd = (pq.flags & 0x0100) != 0;
    uint16_t opcode = pq.flags & 0x7800;
    const char* ip = NULL;
    if (pq.qclass == 1 && pq.qtype == 1) {
        ip = hosts_lookup(hosts, hlen, pq.qname);
    }
    int rcode = ip ? 0 : 3;

    size_t off=0;
    if (outcap < 12) return 0;
    wr16(out,&off,pq.id);
    wr16(out,&off,(uint16_t)(0x8000 | opcode | 0x0400 | (rd?0x0100:0) | (rcode & 0xF)));
    wr16(out,&off,1);
    wr16(out,&off, ip?1:0);
    wr16(out,&off,0);
    wr16(out,&off,0);
    size_t qlen = pq.questionEnd - 12;
    if (off + qlen > outcap) return 0;
    memcpy(out+off, request+12, qlen); off += qlen;
    if (ip){
        if (off + 12 + 4 > outcap) return 0;
        wr16(out,&off,0xC00C);
        wr16(out,&off,1);
        wr16(out,&off,1);
        wr32(out,&off,60);
        wr16(out,&off,4);
        uint8_t ipb[4];
        if (ip_to_bytes(ip, ipb)!=0) return 0;
        memcpy(out+off, ipb, 4); off += 4;
    }
    return off;
}

size_t buildServfail(const uint8_t* request, size_t rlen, uint8_t* out, size_t outcap){
    ParsedQuery pq;
    if (parseDnsQuery(request, rlen, &pq) != 0){
        if (outcap < 12) return 0;
        size_t off=0;
        wr16(out,&off,0); wr16(out,&off,0x8182); wr16(out,&off,0); wr16(out,&off,0); wr16(out,&off,0); wr16(out,&off,0);
        return off;
    }
    size_t off=0;
    wr16(out,&off,pq.id);
    wr16(out,&off,(uint16_t)(0x8000 | (pq.flags & 0x7800) | ((pq.flags & 0x0100)?0x0100:0) | 2));
    wr16(out,&off,1); wr16(out,&off,0); wr16(out,&off,0); wr16(out,&off,0);
    size_t qlen = pq.questionEnd - 12;
    if (off + qlen > outcap) return off;
    memcpy(out+off, request+12, qlen); off += qlen;
    return off;
}

size_t buildDnsQuery(const char* name, uint16_t qtype, int rd, uint8_t* out, size_t cap){
    if (cap < 12) return 0;
    size_t off=0;
    uint16_t id = (uint16_t)(time(NULL) & 0xFFFF);
    wr16(out,&off,id);
    wr16(out,&off, rd ? 0x0100 : 0);
    wr16(out,&off,1); wr16(out,&off,0); wr16(out,&off,0); wr16(out,&off,0);
    // qname
    const char* p = name;
    while (*p){
        const char* dot = strchr(p, '.');
        size_t len = dot ? (size_t)(dot - p) : strlen(p);
        if (off + 1 + len >= cap) return 0;
        out[off++] = (uint8_t)len;
        memcpy(out+off, p, len); off += len;
        p = dot ? dot + 1 : p + len;
        if (!dot) break;
    }
    if (off + 1 + 4 > cap) return 0;
    out[off++] = 0;
    wr16(out,&off,qtype);
    wr16(out,&off,1);
    return off;
}

void runUdpDnsServer(const char* ip, uint16_t port, const kv_t* hosts, size_t hlen, int maxRequests){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    struct sockaddr_in addr; bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0){ close(fd); return; }
    uint8_t buf[1500], out[1500];
    int handled = 0;
    while (maxRequests <= 0 || handled < maxRequests){
        struct sockaddr_in cli; socklen_t clen = sizeof(cli);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*)&cli, &clen);
        if (n <= 0) break;
        size_t olen = buildDnsResponse(buf, (size_t)n, hosts, hlen, out, sizeof(out));
        if (olen == 0) olen = buildServfail(buf, (size_t)n, out, sizeof(out));
        sendto(fd, out, olen, 0, (struct sockaddr*)&cli, clen);
        handled++;
    }
    close(fd);
}

int main(){
    kv_t hosts[] = {
        {"example.com", "93.184.216.34"},
        {"localhost", "127.0.0.1"},
        {"github.com", "140.82.113.3"},
        {"test.local", "10.0.0.1"},
        {"service.internal", "192.168.1.100"}
    };
    const char* tests[5] = {"example.com","unknown.example","github.com","localhost","test.local"};
    for (int i=0;i<5;i++){
        uint8_t q[512], r[1500];
        size_t qlen = buildDnsQuery(tests[i], 1, 1, q, sizeof(q));
        size_t rlen = buildDnsResponse(q, qlen, hosts, sizeof(hosts)/sizeof(hosts[0]), r, sizeof(r));
        printf("Q: %s resp_len: %zu\n", tests[i], rlen);
    }
    // Example: runUdpDnsServer("127.0.0.1", 5353, hosts, sizeof(hosts)/sizeof(hosts[0]), 5);
    return 0;
}