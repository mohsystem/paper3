#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

#define PORT 5353
#define IP_ADDRESS "127.0.0.1"
#define BUFFER_SIZE 512

// Simple key-value store for DNS records
struct DnsRecord {
    char* name;
    char* ip;
};

struct DnsRecord dnsRecords[] = {
    {"test1.local.", "192.168.1.1"},
    {"test2.local.", "192.168.1.2"},
    {"example.com.", "93.184.216.34"},
    {"my-server.net.", "10.0.0.1"},
    {"localhost.", "127.0.0.1"},
    {NULL, NULL} // Sentinel
};

const char* find_ip(const char* name) {
    for (int i = 0; dnsRecords[i].name != NULL; ++i) {
        if (strcmp(dnsRecords[i].name, name) == 0) {
            return dnsRecords[i].ip;
        }
    }
    return NULL;
}

// Parses a domain name from a raw DNS packet buffer.
// Returns the length of the parsed name section in the buffer.
int parse_domain_name(const unsigned char* buffer, int buffer_len, int* pos, char* out_name) {
    int name_pos = 0;
    int current_pos = *pos;
    int len_of_name_in_buffer = 0;
    int one_pass = 1;

    while (current_pos < buffer_len && buffer[current_pos] != 0) {
        unsigned char len = buffer[current_pos];
        if ((len & 0xC0) == 0xC0) { // Pointer
             if (current_pos + 1 >= buffer_len) return -1; // Malformed
            int pointer = ((len & 0x3F) << 8) + buffer[current_pos + 1];
            int dummy_pos = pointer;
            // Recursively call to get the pointed-to name part
            // but append it to our current out_name
            int sub_len = parse_domain_name(buffer, buffer_len, &dummy_pos, out_name + name_pos);
            if (sub_len == -1) return -1;
            
            if (one_pass) len_of_name_in_buffer = (current_pos - *pos) + 2;
            *pos += len_of_name_in_buffer; // Update original pos
            return len_of_name_in_buffer;
        }

        if (current_pos + 1 + len > buffer_len) return -1; // Malformed
        
        memcpy(out_name + name_pos, buffer + current_pos + 1, len);
        name_pos += len;
        out_name[name_pos] = '.';
        name_pos++;
        current_pos += (len + 1);
        one_pass = 0; // It's not a pointer on first try
    }

    out_name[name_pos] = '\0';
    len_of_name_in_buffer = (buffer[current_pos] == 0) ? (current_pos - *pos) + 1 : current_pos - *pos;
    *pos += len_of_name_in_buffer;
    return len_of_name_in_buffer;
}


int build_dns_response(const unsigned char* request, int request_len, unsigned char* response, int* response_len) {
    if (request_len < 12) return -1;

    // --- Parse Request ---
    char domain_name[256];
    int pos = 12;
    if (parse_domain_name(request, request_len, &pos, domain_name) < 0) {
        fprintf(stderr, "Failed to parse domain name.\n");
        return -1;
    }

    if (pos + 4 > request_len) return -1; // Missing QTYPE/QCLASS

    uint16_t qtype = (request[pos] << 8) | request[pos + 1];
    uint16_t qclass = (request[pos + 2] << 8) | request[pos + 3];

    const char* ip_str = find_ip(domain_name);
    if (qtype != 1 || qclass != 1 || ip_str == NULL) {
        printf("Unsupported query or unknown domain: %s\n", domain_name);
        return -1;
    }
    printf("Query for: %s\n", domain_name);

    // --- Build Response ---
    // Copy header
    memcpy(response, request, 12);
    // Set flags (QR=1, AA=1, RCODE=0 -> 0x8400)
    response[2] = 0x84;
    response[3] = 0x00;
    // ANCOUNT=1
    response[6] = 0x00;
    response[7] = 0x01;

    // Copy question section
    int question_len = (pos + 4) - 12;
    memcpy(response + 12, request + 12, question_len);
    int current_len = 12 + question_len;

    // --- Add Answer Section ---
    // Pointer to name
    response[current_len++] = 0xC0;
    response[current_len++] = 0x0C;
    // Type (A), Class (IN)
    response[current_len++] = 0x00; response[current_len++] = 0x01;
    response[current_len++] = 0x00; response[current_len++] = 0x01;
    // TTL (60s)
    uint32_t ttl = htonl(60);
    memcpy(response + current_len, &ttl, 4);
    current_len += 4;
    // RDLENGTH (4)
    response[current_len++] = 0x00; response[current_len++] = 0x04;
    // RDATA
    struct in_addr addr;
    inet_pton(AF_INET, ip_str, &addr);
    memcpy(response + current_len, &addr.s_addr, 4);
    current_len += 4;
    
    *response_len = current_len;
    return 0;
}


void start_dns_server(const char* ip, int port) {
    #ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return;
    }
    #endif

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        perror("socket");
        return;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        perror("bind");
        closesocket(sock);
        return;
    }

    printf("DNS server listening on %s:%d\n", ip, port);

    unsigned char request_buf[BUFFER_SIZE];
    unsigned char response_buf[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        int recv_len = recvfrom(sock, (char*)request_buf, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len > 0) {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            printf("Received request from %s\n", client_ip);

            int response_len = 0;
            if (build_dns_response(request_buf, recv_len, response_buf, &response_len) == 0) {
                sendto(sock, (char*)response_buf, response_len, 0, (struct sockaddr*)&client_addr, client_len);
            }
        }
    }
    closesocket(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
}

int main() {
    printf("Starting C DNS Server...\n");
    printf("You can test this server with commands like:\n");
    printf("  dig @%s -p %d test1.local\n", IP_ADDRESS, PORT);
    printf("  nslookup example.com %s\n", IP_ADDRESS);
    printf("----------------------------------------------\n");
    start_dns_server(IP_ADDRESS, PORT);
    return 0;
}