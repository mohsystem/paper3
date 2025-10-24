
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
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

#define MAX_DNS_RECORDS 10
#define DNS_PORT 5353

typedef struct {
    char domain[256];
    char ip[16];
} DNSRecord;

DNSRecord dnsRecords[MAX_DNS_RECORDS];
int recordCount = 0;

void initDNSRecords() {
    strcpy(dnsRecords[0].domain, "example.com");
    strcpy(dnsRecords[0].ip, "93.184.216.34");
    strcpy(dnsRecords[1].domain, "test.local");
    strcpy(dnsRecords[1].ip, "127.0.0.1");
    strcpy(dnsRecords[2].domain, "localhost");
    strcpy(dnsRecords[2].ip, "127.0.0.1");
    strcpy(dnsRecords[3].domain, "google.com");
    strcpy(dnsRecords[3].ip, "142.250.185.78");
    strcpy(dnsRecords[4].domain, "github.com");
    strcpy(dnsRecords[4].ip, "140.82.121.3");
    recordCount = 5;
}

const char* findIPAddress(const char* domain) {
    for (int i = 0; i < recordCount; i++) {
        if (strcmp(dnsRecords[i].domain, domain) == 0) {
            return dnsRecords[i].ip;
        }
    }
    return "0.0.0.0";
}

void parseDomainName(unsigned char* data, int* offset, char* domain) {
    int pos = 0;
    int length;
    
    while ((length = data[(*offset)++]) != 0) {
        if (pos > 0) domain[pos++] = '.';
        for (int i = 0; i < length; i++) {
            domain[pos++] = data[(*offset)++];
        }
    }
    domain[pos] = '\\0';
}

void encodeDomainName(unsigned char* buffer, int* offset, const char* domain) {
    char temp[256];
    strcpy(temp, domain);
    char* label = strtok(temp, ".");
    
    while (label != NULL) {
        int len = strlen(label);
        buffer[(*offset)++] = len;
        memcpy(&buffer[*offset], label, len);
        *offset += len;
        label = strtok(NULL, ".");
    }
    buffer[(*offset)++] = 0;
}

int buildDNSResponse(unsigned short transactionId, const char* domain, 
                     unsigned short queryType, unsigned short queryClass,
                     unsigned char* response) {
    int offset = 0;
    
    response[offset++] = transactionId >> 8;
    response[offset++] = transactionId & 0xFF;
    response[offset++] = 0x81;
    response[offset++] = 0x80;
    response[offset++] = 0x00;
    response[offset++] = 0x01;
    response[offset++] = 0x00;
    response[offset++] = 0x01;
    response[offset++] = 0x00;
    response[offset++] = 0x00;
    response[offset++] = 0x00;
    response[offset++] = 0x00;
    
    encodeDomainName(response, &offset, domain);
    response[offset++] = queryType >> 8;
    response[offset++] = queryType & 0xFF;
    response[offset++] = queryClass >> 8;
    response[offset++] = queryClass & 0xFF;
    
    encodeDomainName(response, &offset, domain);
    response[offset++] = queryType >> 8;
    response[offset++] = queryType & 0xFF;
    response[offset++] = queryClass >> 8;
    response[offset++] = queryClass & 0xFF;
    response[offset++] = 0x00;
    response[offset++] = 0x00;
    response[offset++] = 0x01;
    response[offset++] = 0x2C;
    
    const char* ipAddress = findIPAddress(domain);
    response[offset++] = 0x00;
    response[offset++] = 0x04;
    
    int a, b, c, d;
    sscanf(ipAddress, "%d.%d.%d.%d", &a, &b, &c, &d);
    response[offset++] = a;
    response[offset++] = b;
    response[offset++] = c;
    response[offset++] = d;
    
    return offset;
}

int processDNSRequest(unsigned char* data, int length, unsigned char* response) {
    unsigned short transactionId = (data[0] << 8) | data[1];
    
    int offset = 12;
    char domainName[256];
    parseDomainName(data, &offset, domainName);
    
    unsigned short queryType = (data[offset] << 8) | data[offset + 1];
    unsigned short queryClass = (data[offset + 2] << 8) | data[offset + 3];
    
    printf("Query for: %s\\n", domainName);
    
    return buildDNSResponse(transactionId, domainName, queryType, queryClass, response);
}

void startDNSServer(const char* ipAddress, int port) {
    initDNSRecords();
    
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
        return;
    }
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress, &serverAddr.sin_addr);
    
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        closesocket(sock);
        return;
    }
    
    printf("DNS Server listening on %s:%d\\n", ipAddress, port);
    
    unsigned char buffer[512];
    unsigned char response[512];
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    while (1) {
        int recvLen = recvfrom(sock, (char*)buffer, sizeof(buffer), 0, 
                              (struct sockaddr*)&clientAddr, &clientLen);
        if (recvLen > 0) {
            int responseLen = processDNSRequest(buffer, recvLen, response);
            sendto(sock, (char*)response, responseLen, 0, 
                   (struct sockaddr*)&clientAddr, clientLen);
            
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            printf("Response sent to %s:%d\\n", clientIP, ntohs(clientAddr.sin_port));
        }
    }
    
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    printf("=== DNS Server Test Cases ===\\n\\n");
    printf("Note: Full testing with separate threads is complex in C.\\n");
    printf("Please run this program and test with dig or nslookup:\\n");
    printf("Example: dig @127.0.0.1 -p 5353 example.com\\n\\n");
    
    startDNSServer("127.0.0.1", DNS_PORT);
    
    return 0;
}
