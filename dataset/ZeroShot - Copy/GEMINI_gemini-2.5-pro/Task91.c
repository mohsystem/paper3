#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define socklen_t int
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Constants
#define PORT 53530
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 512

// Simple DNS record storage
typedef struct {
    const char* name;
    const char* ip;
} DnsRecord;

DnsRecord dns_records[] = {
    {"test1.example.com.", "192.0.2.1"},
    {"test2.example.com.", "198.51.100.2"},
    {"secure.example.com.", "203.0.113.3"},
    {NULL, NULL} // Sentinel
};

// DNS Header structure
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

// Function to parse a domain name from DNS wire format to "www.example.com"
int parse_domain_name(const uint8_t* buffer, int size, int offset, char* out_name, int* bytes_read) {
    char* name_ptr = out_name;
    int original_offset = offset;

    while (offset < size && buffer[offset] != 0) {
        uint8_t length = buffer[offset];
        if ((length & 0xC0) == 0xC0) {
            // Pointers in requests are not supported
            return -1;
        }
        offset++;
        if (offset + length > size) return -1; // Malformed
        
        if (name_ptr != out_name) {
            *name_ptr++ = '.';
        }
        memcpy(name_ptr, &buffer[offset], length);
        name_ptr += length;
        offset += length;
    }
    *name_ptr++ = '.';
    *name_ptr = '\0';
    offset++; // For the null terminator
    
    *bytes_read = offset - original_offset;
    return 0;
}

// Find an IP for a given domain name
const char* find_ip(const char* name) {
    for (int i = 0; dns_records[i].name != NULL; ++i) {
        if (strcmp(name, dns_records[i].name) == 0) {
            return dns_records[i].ip;
        }
    }
    return NULL;
}

void start_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return;
    }
#endif

    SOCKET server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock == INVALID_SOCKET) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP_ADDRESS, &server_addr.sin_addr);

    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("Bind failed");
        closesocket(server_sock);
        return;
    }

    printf("C DNS Server listening on %s:%d\n", IP_ADDRESS, PORT);

    uint8_t buffer[BUFFER_SIZE];
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int bytes_received = recvfrom(server_sock, (char*)buffer, BUFFER_SIZE, 0,
                                      (struct sockaddr*)&client_addr, &client_len);

        if (bytes_received < sizeof(DnsHeader)) continue;

        DnsHeader* header = (DnsHeader*)buffer;
        // Security check: Only one question, standard query
        if (ntohs(header->qdcount) != 1 || (ntohs(header->flags) & 0x7800) != 0) {
            continue;
        }

        char domain_name[256];
        int qname_len;
        if (parse_domain_name(buffer, bytes_received, sizeof(DnsHeader), domain_name, &qname_len) != 0) {
            continue; // Parsing failed
        }
        
        int question_offset = sizeof(DnsHeader) + qname_len;
        uint16_t qtype = ntohs(*(uint16_t*)&buffer[question_offset]);
        uint16_t qclass = ntohs(*(uint16_t*)&buffer[question_offset + 2]);

        const char* ip_address = find_ip(domain_name);

        uint8_t response[BUFFER_SIZE];
        memcpy(response, buffer, sizeof(DnsHeader));
        DnsHeader* response_header = (DnsHeader*)response;
        
        // Copy question section
        int question_total_len = qname_len + 4; // name + type + class
        memcpy(response + sizeof(DnsHeader), buffer + sizeof(DnsHeader), question_total_len);
        int response_len = sizeof(DnsHeader) + question_total_len;

        if (ip_address != NULL && qtype == 1 && qclass == 1) { // A, IN
            response_header->flags = htons(0x8180); // Response, no error
            response_header->ancount = htons(1);

            // Answer section
            uint16_t* name_ptr = (uint16_t*)(response + response_len);
            *name_ptr = htons(0xC00C); // Pointer to name at offset 12
            response_len += 2;

            uint16_t* type_ptr = (uint16_t*)(response + response_len);
            *type_ptr = htons(1); // Type A
            response_len += 2;

            uint16_t* class_ptr = (uint16_t*)(response + response_len);
            *class_ptr = htons(1); // Class IN
            response_len += 2;

            uint32_t* ttl_ptr = (uint32_t*)(response + response_len);
            *ttl_ptr = htonl(60); // TTL 60
            response_len += 4;

            uint16_t* rdlen_ptr = (uint16_t*)(response + response_len);
            *rdlen_ptr = htons(4); // Length of IPv4
            response_len += 2;

            uint32_t* rdata_ptr = (uint32_t*)(response + response_len);
            inet_pton(AF_INET, ip_address, rdata_ptr);
            response_len += 4;
        } else { // Not found or unsupported
            response_header->flags = htons(0x8183); // Name Error
            response_header->ancount = htons(0);
        }

        sendto(server_sock, (char*)response, response_len, 0, (struct sockaddr*)&client_addr, client_len);
    }

    closesocket(server_sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    printf("--- C Simple DNS Server ---\n");
    printf("This server will run indefinitely. Stop with Ctrl+C.\n");
    printf("--- 5 Test Cases to run manually ---\n");
    printf("1. Known domain:      dig @127.0.0.1 -p 53530 test1.example.com A\n");
    printf("2. Known domain:      dig @127.0.0.1 -p 53530 test2.example.com A\n");
    printf("3. Known domain:      dig @127.0.0.1 -p 53530 secure.example.com A\n");
    printf("4. Unknown domain:    dig @127.0.0.1 -p 53530 unknown.example.com A\n");
    printf("5. Unsupported type:  dig @127.0.0.1 -p 53530 test1.example.com MX\n");
    printf("----------------------------------\n");
    
    start_server();
    
    return 0;
}