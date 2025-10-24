
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
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

typedef struct {
    char domain[256];
    char ip[16];
} DNSRecord;

DNSRecord DNS_RECORDS[] = {
    {"example.com", "93.184.216.34"},
    {"test.com", "192.168.1.100"},
    {"localhost", "127.0.0.1"},
    {"google.com", "142.250.185.46"},
    {"github.com", "140.82.114.4"}
};

#define DNS_RECORDS_COUNT 5

void startDNSServer(const char* ipAddress, int port) {
    #ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    #endif
    
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Failed to create socket\\n");
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
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    
    int recvLen = recvfrom(sock, (char*)buffer, 512, 0, 
                           (struct sockaddr*)&clientAddr, &clientLen);
    
    if (recvLen > 0) {
        char domainName[256];
        parseDNSQuery(buffer, recvLen, domainName);
        printf("Received query for: %s\\n", domainName);
        
        char ipAddr[16];
        resolveDNSRecord(domainName, ipAddr);
        
        unsigned char response[512];
        int responseLen = createDNSResponse(buffer, recvLen, ipAddr, response);
        
        sendto(sock, (char*)response, responseLen, 0,
               (struct sockaddr*)&clientAddr, clientLen);
        printf("Sent response: %s\\n", ipAddr);
    }
    
    closesocket(sock);
    
    #ifdef _WIN32
    WSACleanup();
    #endif
}

void parseDNSQuery(const unsigned char* query, int queryLen, char* domain) {
    domain[0] = '\\0';
    int pos = 12; // Skip DNS header
    char temp[256] = "";
    
    while (pos < queryLen && query[pos] != 0) {
        int length = query[pos];
        if (length == 0) break;
        
        if (strlen(temp) > 0) strcat(temp, ".");
        pos++;
        
        for (int i = 0; i < length && pos < queryLen; i++, pos++) {
            char c[2] = {(char)query[pos], '\\0'};
            strcat(temp, c);
        }
    }
    
    strcpy(domain, temp);
}

void resolveDNSRecord(const char* domainName, char* ipAddress) {
    strcpy(ipAddress, "0.0.0.0");
    
    for (int i = 0; i < DNS_RECORDS_COUNT; i++) {
        if (strcmp(DNS_RECORDS[i].domain, domainName) == 0) {
            strcpy(ipAddress, DNS_RECORDS[i].ip);
            return;
        }
    }
}

int createDNSResponse(const unsigned char* query, int queryLen, 
                      const char* ipAddress, unsigned char* response) {
    int pos = 0;
    
    // Copy transaction ID
    response[pos++] = query[0];
    response[pos++] = query[1];
    
    // Flags: Response, Standard Query, No Error
    response[pos++] = 0x81;
    response[pos++] = 0x80;
    
    // Questions count
    response[pos++] = query[4];
    response[pos++] = query[5];
    
    // Answers count
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    
    // Authority and Additional RRs
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    
    // Copy question section
    int qpos = 12;
    while (qpos < queryLen && query[qpos] != 0) {
        response[pos++] = query[qpos++];
    }
    response[pos++] = 0x00;
    qpos++;
    
    // Copy QTYPE and QCLASS
    for (int i = 0; i < 4 && qpos < queryLen; i++, qpos++) {
        response[pos++] = query[qpos];
    }
    
    // Answer section
    response[pos++] = 0xC0;
    response[pos++] = 0x0C;
    
    // Type A
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    
    // Class IN
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    
    // TTL (300 seconds)
    response[pos++] = 0x00;
    response[pos++] = 0x00;
    response[pos++] = 0x01;
    response[pos++] = 0x2C;
    
    // Data length
    response[pos++] = 0x00;
    response[pos++] = 0x04;
    
    // IP Address
    int a, b, c, d;
    sscanf(ipAddress, "%d.%d.%d.%d", &a, &b, &c, &d);
    response[pos++] = (unsigned char)a;
    response[pos++] = (unsigned char)b;
    response[pos++] = (unsigned char)c;
    response[pos++] = (unsigned char)d;
    
    return pos;
}

void createTestQuery(const char* domain, unsigned char* query, int* queryLen) {
    int pos = 0;
    
    query[pos++] = 0x12;
    query[pos++] = 0x34;
    query[pos++] = 0x01;
    query[pos++] = 0x00;
    query[pos++] = 0x00;
    query[pos++] = 0x01;
    for (int i = 0; i < 6; i++) query[pos++] = 0x00;
    
    char domainCopy[256];
    strcpy(domainCopy, domain);
    
    char* label = strtok(domainCopy, ".");
    while (label != NULL) {
        int len = strlen(label);
        query[pos++] = (unsigned char)len;
        for (int i = 0; i < len; i++) {
            query[pos++] = (unsigned char)label[i];
        }
        label = strtok(NULL, ".");
    }
    
    query[pos++] = 0x00;
    query[pos++] = 0x00;
    query[pos++] = 0x01;
    query[pos++] = 0x00;
    query[pos++] = 0x01;
    
    *queryLen = pos;
}

int main() {
    printf("DNS Server Test Cases:\\n");
    printf("======================\\n\\n");
    
    // Test Case 1: Parse DNS query
    printf("Test 1: Parse DNS Query\\n");
    unsigned char testQuery1[512];
    int queryLen1;
    createTestQuery("example.com", testQuery1, &queryLen1);
    char parsed1[256];
    parseDNSQuery(testQuery1, queryLen1, parsed1);
    printf("Parsed domain: %s\\n\\n", parsed1);
    
    // Test Case 2: Resolve DNS record
    printf("Test 2: Resolve DNS Record\\n");
    char resolved[16];
    resolveDNSRecord("google.com", resolved);
    printf("Resolved IP: %s\\n\\n", resolved);
    
    // Test Case 3: Resolve unknown domain
    printf("Test 3: Resolve Unknown Domain\\n");
    char unknownResolved[16];
    resolveDNSRecord("unknown.com", unknownResolved);
    printf("Resolved IP: %s\\n\\n", unknownResolved);
    
    // Test Case 4: Create DNS response
    printf("Test 4: Create DNS Response\\n");
    unsigned char response[512];
    int responseLen = createDNSResponse(testQuery1, queryLen1, "93.184.216.34", response);
    printf("Response created with length: %d\\n\\n", responseLen);
    
    // Test Case 5: Multiple domain parsing
    printf("Test 5: Parse Multiple Domains\\n");
    const char* domains[] = {"localhost", "test.com", "github.com"};
    for (int i = 0; i < 3; i++) {
        unsigned char query[512];
        int queryLen;
        createTestQuery(domains[i], query, &queryLen);
        char parsed[256];
        parseDNSQuery(query, queryLen, parsed);
        char ip[16];
        resolveDNSRecord(parsed, ip);
        printf("%s -> %s -> %s\\n", domains[i], parsed, ip);
    }
    
    return 0;
}
