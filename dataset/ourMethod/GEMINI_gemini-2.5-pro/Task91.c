#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Platform-specific socket headers
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define socklen_t int
#define SSIZE_T int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define closesocket close
#endif

// Define constants
#define DNS_PORT 53535
#define LISTEN_ADDR "127.0.0.1"
#define BUFFER_SIZE 512

// A simple key-value pair for our DNS records
typedef struct {
    const char* name;
    const char* ip;
} DnsRecord;

static const DnsRecord dns_records[] = {
    {"example.com.", "93.184.216.34"},
    {"test.local.", "192.168.1.100"},
    {"hello.world.", "1.1.1.1"},
    {"another.test.", "8.8.8.8"},
    {"localhost.", "127.0.0.1"},
    {NULL, NULL} // Sentinel
};

#pragma pack(push, 1)
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} DnsHeader;
#pragma pack(pop)

// Parses a domain name from DNS format. Returns length of the parsed name section.
size_t parse_domain_name(const unsigned char* buffer, size_t buffer_len, size_t offset, char* out_name, size_t out_name_size) {
    size_t name_pos = 0;
    size_t current_offset = offset;
    if (offset >= buffer_len) return 0;
    
    while (current_offset < buffer_len && buffer[current_offset] != 0) {
        if ((buffer[current_offset] & 0xC0) == 0xC0) {
            fprintf(stderr, "Pointers in request QNAME are not supported.\n");
            return 0;
        }
        uint8_t length = buffer[current_offset++];
        if (current_offset + length > buffer_len || name_pos + length + 1 >= out_name_size) {
            fprintf(stderr, "Malformed QNAME or output buffer too small.\n");
            return 0;
        }
        memcpy(out_name + name_pos, buffer + current_offset, length);
        name_pos += length;
        current_offset += length;
        if (current_offset < buffer_len && buffer[current_offset] != 0) {
            out_name[name_pos++] = '.';
        }
    }
    
    if (current_offset >= buffer_len || name_pos + 2 >= out_name_size) return 0;
    current_offset++; // Move past the null terminator
    out_name[name_pos++] = '.';
    out_name[name_pos] = '\0';
    return current_offset - offset;
}

const char* resolve_name(const char* name) {
    for (int i = 0; dns_records[i].name != NULL; i++) {
        if (strcmp(name, dns_records[i].name) == 0) {
            return dns_records[i].ip;
        }
    }
    return NULL;
}

int main() {
    // Test cases: use a DNS client like `dig` or `nslookup` against this server.
    // 1. dig @127.0.0.1 -p 53535 example.com
    // 2. dig @127.0.0.1 -p 53535 test.local
    // 3. dig @127.0.0.1 -p 53535 hello.world
    // 4. dig @127.0.0.1 -p 53535 not.found
    // 5. dig @127.0.0.1 -p 53535 localhost
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n"); return 1;
    }
#endif
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) { perror("socket creation failed"); return 1; }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, LISTEN_ADDR, &server_addr.sin_addr);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        closesocket(sock);
        return 1;
    }
    printf("Starting C DNS server on %s:%d\n", LISTEN_ADDR, DNS_PORT);

    while (1) {
        unsigned char buffer[BUFFER_SIZE];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // 1. Receive incoming DNS request
        SSIZE_T bytes_received = recvfrom(sock, (char*)buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (bytes_received < 0) { perror("recvfrom failed"); continue; }
        
        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        printf("\nReceived request from %s:%d\n", client_ip_str, ntohs(client_addr.sin_port));

        // 2. Parse the received data
        if ((size_t)bytes_received < sizeof(DnsHeader)) continue;
        DnsHeader* header = (DnsHeader*)buffer;
        if (ntohs(header->qdcount) != 1) continue;

        char domain_name[256];
        size_t question_name_len = parse_domain_name(buffer, bytes_received, sizeof(DnsHeader), domain_name, sizeof(domain_name));
        if (question_name_len == 0) continue;

        size_t question_offset = sizeof(DnsHeader) + question_name_len;
        if (question_offset + 4 > (size_t)bytes_received) continue;
        uint16_t qtype = (buffer[question_offset] << 8) | buffer[question_offset + 1];
        uint16_t qclass = (buffer[question_offset + 2] << 8) | buffer[question_offset + 3];

        if (qtype != 1 || qclass != 1) continue;
        printf("Query for: %s (A, IN)\n", domain_name);

        // 3. Resolve record & 4. Construct response
        unsigned char response[BUFFER_SIZE];
        size_t question_len = question_name_len + 4;
        size_t response_len = sizeof(DnsHeader) + question_len;
        if (response_len > BUFFER_SIZE) continue;
        memcpy(response, buffer, response_len);

        DnsHeader* res_header = (DnsHeader*)response;
        const char* resolved_ip = resolve_name(domain_name);

        if (resolved_ip) {
            printf("Resolved %s to %s\n", domain_name, resolved_ip);
            res_header->flags = htons(0x8180); // Response, no error
            res_header->ancount = htons(1);
            if (response_len + 16 > BUFFER_SIZE) continue; // Check space for answer
            response[response_len++] = 0xc0; response[response_len++] = 0x0c; // Pointer
            response[response_len++] = 0x00; response[response_len++] = 0x01; // Type A
            response[response_len++] = 0x00; response[response_len++] = 0x01; // Class IN
            uint32_t ttl_n = htonl(60);
            memcpy(response + response_len, &ttl_n, 4); response_len += 4;
            uint16_t rdlen_n = htons(4);
            memcpy(response + response_len, &rdlen_n, 2); response_len += 2;
            struct in_addr ip_addr;
            inet_pton(AF_INET, resolved_ip, &ip_addr);
            memcpy(response + response_len, &ip_addr, 4); response_len += 4;
        } else {
            printf("Could not resolve: %s\n", domain_name);
            res_header->flags = htons(0x8183); // Name Error
            res_header->ancount = htons(0);
        }

        // 5. Send response
        sendto(sock, (char*)response, response_len, 0, (struct sockaddr*)&client_addr, client_len);
        printf("Sent response to %s:%d\n", client_ip_str, ntohs(client_addr.sin_port));
    }
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}