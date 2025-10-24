#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>


#pragma pack(push, 1)
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} DnsHeader;

typedef struct {
    uint16_t qtype;
    uint16_t qclass;
} DnsQuestionTrailer;

typedef struct {
    uint16_t name;
    uint16_t type;
    uint16_t a_class;
    uint32_t ttl;
    uint16_t rdlength;
    uint32_t rdata;
} DnsAnswer;
#pragma pack(pop)

const int PORT = 5353;
const char* IP = "127.0.0.1";

// Simple key-value store for DNS records
const char* dns_records_keys[] = {"test1.dns.local", "test2.dns.local", "test3.dns.local", NULL};
const char* dns_records_values[] = {"1.1.1.1", "2.2.2.2", "3.3.3.3", NULL};

const char* find_ip(const char* domain) {
    for (int i = 0; dns_records_keys[i] != NULL; ++i) {
        if (strcmp(domain, dns_records_keys[i]) == 0) {
            return dns_records_values[i];
        }
    }
    return NULL;
}

void parse_domain_name(const uint8_t* buffer, int* offset, char* out_name) {
    int name_pos = 0;
    while (buffer[*offset] != 0) {
        uint8_t len = buffer[(*offset)++];
        if (name_pos > 0) {
            out_name[name_pos++] = '.';
        }
        memcpy(out_name + name_pos, buffer + *offset, len);
        name_pos += len;
        *offset += len;
    }
    out_name[name_pos] = '\0';
    (*offset)++; // Skip the null terminator
}

void encode_domain_name(uint8_t* buffer, int* offset, const char* domain) {
    int start = 0;
    for (int i = 0; ; ++i) {
        if (domain[i] == '.' || domain[i] == '\0') {
            buffer[(*offset)++] = i - start;
            memcpy(buffer + *offset, domain + start, i - start);
            *offset += (i - start);
            start = i + 1;
            if (domain[i] == '\0') break;
        }
    }
    buffer[(*offset)++] = 0;
}

void run_server() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &servaddr.sin_addr);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    printf("C DNS Server listening on %s:%d\n", IP, PORT);
    
    uint8_t buffer[512];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);

    while (1) {
        int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&cliaddr, &len);
        if (n <= 0) continue;

        DnsHeader* req_header = (DnsHeader*)buffer;
        int offset = sizeof(DnsHeader);
        char domain_name[256];
        parse_domain_name(buffer, &offset, domain_name);
        DnsQuestionTrailer* q_trailer = (DnsQuestionTrailer*)(buffer + offset);

        uint8_t response_buffer[512];
        DnsHeader* res_header = (DnsHeader*)response_buffer;
        res_header->id = req_header->id;
        res_header->qdcount = htons(1);
        res_header->nscount = 0;
        res_header->arcount = 0;

        int response_size = sizeof(DnsHeader);
        encode_domain_name(response_buffer, &response_size, domain_name);
        
        DnsQuestionTrailer* res_q_trailer = (DnsQuestionTrailer*)(response_buffer + response_size);
        res_q_trailer->qtype = q_trailer->qtype;
        res_q_trailer->qclass = q_trailer->qclass;
        response_size += sizeof(DnsQuestionTrailer);

        const char* ip_str = find_ip(domain_name);
        if (ip_str != NULL && ntohs(q_trailer->qtype) == 1) { // A record
             res_header->flags = htons(0x8180);
             res_header->ancount = htons(1);
             
             DnsAnswer* answer = (DnsAnswer*)(response_buffer + response_size);
             answer->name = htons(0xC00C);
             answer->type = htons(1);
             answer->a_class = htons(1);
             answer->ttl = htonl(60);
             answer->rdlength = htons(4);
             inet_pton(AF_INET, ip_str, &answer->rdata);
             response_size += sizeof(DnsAnswer);
        } else { // NXDOMAIN
            res_header->flags = htons(0x8183);
            res_header->ancount = 0;
        }

        sendto(sockfd, response_buffer, response_size, 0, (const struct sockaddr *)&cliaddr, len);
    }
    close(sockfd);
}

void run_client_test(const char* hostname) {
    printf("\n--- Testing: %s ---\n", hostname);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &servaddr.sin_addr);
    
    uint8_t query_buffer[512];
    memset(query_buffer, 0, sizeof(query_buffer));
    DnsHeader* header = (DnsHeader*)query_buffer;
    header->id = htons(0x1234);
    header->flags = htons(0x0100);
    header->qdcount = htons(1);

    int offset = sizeof(DnsHeader);
    encode_domain_name(query_buffer, &offset, hostname);

    DnsQuestionTrailer* q_trailer = (DnsQuestionTrailer*)(query_buffer + offset);
    q_trailer->qtype = htons(1); // A
    q_trailer->qclass = htons(1); // IN
    offset += sizeof(DnsQuestionTrailer);
    
    sendto(sockfd, query_buffer, offset, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    
    uint8_t response_buffer[512];
    int n = recvfrom(sockfd, response_buffer, sizeof(response_buffer), 0, NULL, NULL);

    printf("Query sent for: %s\n", hostname);
    if (n > 0) {
        DnsHeader* res_header = (DnsHeader*)response_buffer;
        uint16_t flags = ntohs(res_header->flags);
        printf("Flags: 0x%04x\n", flags);
        if ((flags & 0x000F) == 3) {
            printf("Response: NXDOMAIN (Not Found)\n");
        } else if (ntohs(res_header->ancount) > 0) {
            DnsAnswer* ans = (DnsAnswer*)(response_buffer + n - sizeof(DnsAnswer));
            struct in_addr ip_addr;
            ip_addr.s_addr = ans->rdata;
            printf("Resolved IP: %s\n", inet_ntoa(ip_addr));
        }
    } else {
        printf("No response from server.\n");
    }
    close(sockfd);
}

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: Server
        run_server();
        exit(EXIT_SUCCESS);
    } else {
        // Parent process: Client
        sleep(1); // Give server time to start

        // 5 Test cases
        run_client_test("test1.dns.local");
        run_client_test("test2.dns.local");
        run_client_test("unknown.dns.local");
        run_client_test("test3.dns.local");
        run_client_test("another.unknown.com");
        
        // Kill the server process and wait for it to terminate
        kill(pid, SIGTERM);
        wait(NULL);
        printf("\nAll tests completed.\n");
    }
    return 0;
}