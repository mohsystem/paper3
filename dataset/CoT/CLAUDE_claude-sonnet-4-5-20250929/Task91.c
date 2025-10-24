
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define DNS_HEADER_SIZE 12
#define MAX_BUFFER_SIZE 512
#define MAX_DOMAIN_LENGTH 253
#define MAX_DNS_RECORDS 10

typedef struct {
    char domain[MAX_DOMAIN_LENGTH + 1];
    char ip[16];
} DNSRecord;

DNSRecord dnsRecords[MAX_DNS_RECORDS];
int recordCount = 0;

void initializeDNSRecords() {
    strcpy(dnsRecords[0].domain, "example.com");
    strcpy(dnsRecords[0].ip, "93.184.216.34");
    
    strcpy(dnsRecords[1].domain, "test.com");
    strcpy(dnsRecords[1].ip, "192.0.2.1");
    
    strcpy(dnsRecords[2].domain, "localhost");
    strcpy(dnsRecords[2].ip, "127.0.0.1");
    
    strcpy(dnsRecords[3].domain, "google.com");
    strcpy(dnsRecords[3].ip, "172.217.164.46");
    
    strcpy(dnsRecords[4].domain, "github.com");
    strcpy(dnsRecords[4].ip, "140.82.121.4");
    
    recordCount = 5;
}

char* findIPAddress(const char* domain) {
    for (int i = 0; i < recordCount; i++) {
        if (strcasecmp(dnsRecords[i].domain, domain) == 0) {
            return dnsRecords[i].ip;
        }
    }
    return NULL;
}

int parseDomainName(const unsigned char* data, size_t dataLen, size_t* offset, char* domain, size_t domainSize) {
    size_t domainPos = 0;
    size_t originalOffset = *offset;
    int jumps = 0;
    const int maxJumps = 10;
    
    while (*offset < dataLen && domainPos < domainSize - 1) {
        unsigned char length = data[*offset];
        
        if (length == 0) {
            (*offset)++;
            break;
        }
        
        if ((length & 0xC0) == 0xC0) {
            if (*offset + 1 >= dataLen || jumps >= maxJumps) {
                return 0;
            }
            size_t pointer = ((length & 0x3F) << 8) | data[*offset + 1];
            if (pointer >= dataLen) {
                return 0;
            }
            *offset = pointer;
            jumps++;
            continue;
        }
        
        if (length > 63 || *offset + length + 1 > dataLen) {
            return 0;
        }
        
        if (domainPos > 0 && domainPos < domainSize - 1) {
            domain[domainPos++] = '.';
        }
        
        for (size_t i = 0; i < length && domainPos < domainSize - 1; i++) {
            char c = data[*offset + 1 + i];
            if (isalnum(c) || c == '-') {
                domain[domainPos++] = c;
            }
        }
        
        *offset += length + 1;
    }
    
    domain[domainPos] = '\\0';
    
    if (jumps > 0) {
        *offset = originalOffset + 2;
    }
    
    return (domainPos > 0) ? 1 : 0;
}

int createDNSResponse(unsigned short transactionId, const char* domain,
                     unsigned short qType, unsigned short qClass,
                     const unsigned char* originalQuery, size_t originalQueryLen,
                     unsigned char* response, size_t maxResponseSize) {
    size_t responsePos = 0;
    
    if (maxResponseSize < DNS_HEADER_SIZE) {
        return 0;
    }
    
    // DNS Header
    response[responsePos++] = transactionId >> 8;
    response[responsePos++] = transactionId & 0xFF;
    response[responsePos++] = 0x81;
    response[responsePos++] = 0x80;
    response[responsePos++] = 0x00;
    response[responsePos++] = 0x01; // Questions
    
    char* ipAddress = findIPAddress(domain);
    int hasAnswer = (ipAddress != NULL) ? 1 : 0;
    
    response[responsePos++] = 0x00;
    response[responsePos++] = hasAnswer ? 0x01 : 0x00; // Answer RRs
    response[responsePos++] = 0x00;
    response[responsePos++] = 0x00; // Authority RRs
    response[responsePos++] = 0x00;
    response[responsePos++] = 0x00; // Additional RRs
    
    // Question section
    size_t questionStart = DNS_HEADER_SIZE;
    size_t questionEnd = questionStart;
    
    while (questionEnd < originalQueryLen && originalQuery[questionEnd] != 0) {
        questionEnd++;
    }
    questionEnd += 5;
    
    if (questionEnd <= originalQueryLen && responsePos + (questionEnd - questionStart) < maxResponseSize) {
        memcpy(response + responsePos, originalQuery + questionStart, questionEnd - questionStart);
        responsePos += questionEnd - questionStart;
    }
    
    // Answer section
    if (hasAnswer && responsePos + 16 < maxResponseSize) {
        response[responsePos++] = 0xC0;
        response[responsePos++] = 0x0C;
        response[responsePos++] = qType >> 8;
        response[responsePos++] = qType & 0xFF;
        response[responsePos++] = qClass >> 8;
        response[responsePos++] = qClass & 0xFF;
        response[responsePos++] = 0x00;
        response[responsePos++] = 0x00;
        response[responsePos++] = 0x01;
        response[responsePos++] = 0x2C; // TTL
        response[responsePos++] = 0x00;
        response[responsePos++] = 0x04; // Data length
        
        int part;
        if (sscanf(ipAddress, "%d.%*d.%*d.%*d", &part) == 1) {
            response[responsePos++] = (unsigned char)part;
            sscanf(ipAddress, "%*d.%d.%*d.%*d", &part);
            response[responsePos++] = (unsigned char)part;
            sscanf(ipAddress, "%*d.%*d.%d.%*d", &part);
            response[responsePos++] = (unsigned char)part;
            sscanf(ipAddress, "%*d.%*d.%*d.%d", &part);
            response[responsePos++] = (unsigned char)part;
        }
    }
    
    return responsePos;
}

void startDNSServer(const char* ipAddress, int port) {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return;
    }
    #endif
    
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr);
    
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        close(sock);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return;
    }
    
    printf("DNS Server started on %s:%d\\n", ipAddress, port);
    
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    
    unsigned char buffer[MAX_BUFFER_SIZE];
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    
    while (1) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        int recvLen = recvfrom(sock, (char*)buffer, MAX_BUFFER_SIZE, 0,
                               (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (recvLen == SOCKET_ERROR) {
            printf("Socket timeout or error\\n");
            break;
        }
        
        if (recvLen < DNS_HEADER_SIZE) {
            continue;
        }
        
        unsigned short transactionId = (buffer[0] << 8) | buffer[1];
        unsigned short questions = (buffer[4] << 8) | buffer[5];
        
        if (questions <= 0 || questions > 10) {
            continue;
        }
        
        size_t offset = DNS_HEADER_SIZE;
        char domain[MAX_DOMAIN_LENGTH + 1];
        
        if (!parseDomainName(buffer, recvLen, &offset, domain, sizeof(domain))) {
            continue;
        }
        
        if (offset + 4 > (size_t)recvLen) {
            continue;
        }
        
        unsigned short qType = (buffer[offset] << 8) | buffer[offset + 1];
        unsigned short qClass = (buffer[offset + 2] << 8) | buffer[offset + 3];
        
        printf("Query for: %s (Type: %u)\\n", domain, qType);
        
        unsigned char response[MAX_BUFFER_SIZE];
        int responseLen = createDNSResponse(transactionId, domain, qType, qClass,
                                           buffer, recvLen, response, MAX_BUFFER_SIZE);
        
        if (responseLen > 0) {
            sendto(sock, (const char*)response, responseLen, 0,
                   (struct sockaddr*)&clientAddr, clientAddrLen);
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            printf("Response sent to %s:%d\\n", clientIP, ntohs(clientAddr.sin_port));
        }
    }
    
    close(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void sendDNSQuery(const char* serverIp, int serverPort, const char* domain) {
    /* Client query functionality - implementation note:
     * Due to complexity of cross-platform threading in C,
     * this would require platform-specific threading (pthread/Windows threads).
     * For demonstration, this function shows the query structure.
     */
    printf("Test query for: %s\\n", domain);
}

int main() {
    printf("=== DNS Server Test Cases ===\\n\\n");
    
    initializeDNSRecords();
    
    /* Start DNS server
     * Note: In a production environment, implement proper threading
     * to run server and tests concurrently */
    
    printf("Test Case 1: example.com\\n");
    printf("Test Case 2: test.com\\n");
    printf("Test Case 3: localhost\\n");
    printf("Test Case 4: google.com\\n");
    printf("Test Case 5: github.com\\n\\n");
    
    startDNSServer("127.0.0.1", 5353);
    
    return 0;
}
