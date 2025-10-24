
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

/* Maximum buffer size to prevent excessive memory usage */
#define MAX_DNS_PACKET_SIZE 512
#define MAX_DOMAIN_LENGTH 253
#define DNS_QUERY_TYPE_A 1
#define DNS_CLASS_IN 1

/* DNS Header structure */
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} DNSHeader;

/* Parse domain name from DNS query, returns length parsed or -1 on error
   Security: bounds checking to prevent buffer overflow (CWE-119, CWE-125) */
int parseDomainName(const uint8_t* buffer, size_t bufferLen, size_t offset,
                    char* domain, size_t domainMaxLen) {
    /* Validate inputs - treat all inputs as untrusted */
    if (buffer == NULL || domain == NULL || bufferLen == 0 || domainMaxLen == 0) {
        return -1;
    }
    
    /* Initialize output buffer to ensure null termination */
    memset(domain, 0, domainMaxLen);
    
    size_t pos = offset;
    size_t domainPos = 0;
    
    while (pos < bufferLen) {
        /* Bounds check before reading length byte */
        if (pos >= bufferLen) {
            return -1;
        }
        
        uint8_t len = buffer[pos];
        
        /* Check for compression pointer (not supported in this implementation) */
        if ((len & 0xC0) == 0xC0) {
            return -1; /* Reject compression to prevent complexity attacks */
        }
        
        if (len == 0) {
            pos++;
            break;
        }
        
        /* Validate label length - DNS label max is 63 bytes */
        if (len > 63) {
            return -1;
        }
        
        pos++;
        
        /* Bounds check: ensure we don't read beyond buffer */\n        if (pos + len > bufferLen) {\n            return -1;\n        }\n        \n        /* Add dot separator if not first label */\n        if (domainPos > 0) {\n            /* Check space for dot and null terminator */\n            if (domainPos + 1 >= domainMaxLen) {\n                return -1;\n            }\n            domain[domainPos++] = '.';\n        }\n        \n        /* Check if we have space for this label plus null terminator */\n        if (domainPos + len >= domainMaxLen) {\n            return -1;\n        }\n        \n        /* Safe copy with bounds checking and character validation */\n        for (size_t i = 0; i < len; i++) {\n            if (pos + i >= bufferLen) {\n                return -1;\n            }\n            \n            char c = (char)buffer[pos + i];\n            \n            /* Validate DNS label characters (alphanumeric, hyphen, underscore) */\n            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||\n                  (c >= '0' && c <= '9') || c == '-' || c == '_')) {\n                return -1;\n            }\n            \n            domain[domainPos++] = c;\n        }\n        \n        pos += len;\n        \n        /* Check total domain length */\n        if (domainPos > MAX_DOMAIN_LENGTH) {\n            return -1;\n        }\n    }\n    \n    /* Ensure null termination */\n    domain[domainPos] = '\\0';\n    \n    return (int)(pos - offset);\n}\n\n/* Build DNS response with bounds checking\n   Security: prevent buffer overflow (CWE-787) */\nint buildDNSResponse(const uint8_t* query, size_t queryLen,\n                     const char* resolvedIP,\n                     uint8_t* response, size_t responseMaxLen,\n                     size_t* responseLen) {\n    /* Validate all inputs */\n    if (query == NULL || resolvedIP == NULL || response == NULL ||\n        responseLen == NULL || queryLen == 0 || responseMaxLen == 0) {\n        return 0;\n    }\n    \n    /* Check minimum query length */\n    if (queryLen < sizeof(DNSHeader) + 6) {\n        return 0;\n    }\n    \n    /* Validate query size */\n    if (queryLen > MAX_DNS_PACKET_SIZE) {\n        return 0;\n    }\n    \n    /* Check if response buffer is large enough */\n    if (responseMaxLen < queryLen + 16) { /* 16 bytes for answer section */\n        return 0;\n    }\n    \n    /* Safe copy with bounds checking */\n    memcpy(response, query, queryLen);\n    size_t pos = queryLen;\n    \n    /* Modify header flags */\n    DNSHeader* header = (DNSHeader*)response;\n    header->flags = htons(0x8180); /* Standard response, no error */\n    header->ancount = htons(1); /* One answer */\n    \n    /* Add answer section with bounds checking */\n    if (pos + 2 > responseMaxLen) return 0;\n    response[pos++] = 0xC0; /* Compression pointer */\n    response[pos++] = 0x0C; /* Offset to question name */\n    \n    /* Type A */\n    if (pos + 2 > responseMaxLen) return 0;\n    response[pos++] = 0x00;\n    response[pos++] = DNS_QUERY_TYPE_A;\n    \n    /* Class IN */\n    if (pos + 2 > responseMaxLen) return 0;\n    response[pos++] = 0x00;\n    response[pos++] = DNS_CLASS_IN;\n    \n    /* TTL (300 seconds) */\n    if (pos + 4 > responseMaxLen) return 0;\n    response[pos++] = 0x00;\n    response[pos++] = 0x00;\n    response[pos++] = 0x01;\n    response[pos++] = 0x2C;\n    \n    /* Data length (4 bytes for IPv4) */\n    if (pos + 2 > responseMaxLen) return 0;\n    response[pos++] = 0x00;\n    response[pos++] = 0x04;\n    \n    /* Parse and add IP address with validation */\n    struct in_addr addr;\n    if (inet_pton(AF_INET, resolvedIP, &addr) != 1) {\n        return 0;\n    }\n    \n    if (pos + 4 > responseMaxLen) return 0;\n    uint32_t ip = ntohl(addr.s_addr);\n    response[pos++] = (ip >> 24) & 0xFF;\n    response[pos++] = (ip >> 16) & 0xFF;\n    response[pos++] = (ip >> 8) & 0xFF;\n    response[pos++] = ip & 0xFF;\n    \n    *responseLen = pos;\n    return 1;\n}\n\n/* Resolve DNS query using getaddrinfo (system resolver)\n   Security: uses system resolver, validates input domain length */\nint resolveDomain(const char* domain, char* ipAddr, size_t ipAddrLen) {\n    /* Validate inputs */\n    if (domain == NULL || ipAddr == NULL || ipAddrLen == 0) {\n        return 0;\n    }\n    \n    /* Initialize output buffer */\n    memset(ipAddr, 0, ipAddrLen);\n    \n    /* Validate domain length */\n    size_t domainLen = strlen(domain);\n    if (domainLen == 0 || domainLen > MAX_DOMAIN_LENGTH) {\n        return 0;\n    }\n    \n    struct addrinfo hints;\n    struct addrinfo* result = NULL;\n    \n    memset(&hints, 0, sizeof(hints));\n    hints.ai_family = AF_INET;\n    hints.ai_socktype = SOCK_DGRAM;\n    \n    /* Use getaddrinfo with validated domain name */\n    int ret = getaddrinfo(domain, NULL, &hints, &result);\n    if (ret != 0 || result == NULL) {\n        if (result != NULL) {\n            freeaddrinfo(result);\n        }\n        return 0;\n    }\n    \n    /* Extract IP address with bounds checking */\n    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;\n    if (inet_ntop(AF_INET, &(addr->sin_addr), ipAddr, ipAddrLen) == NULL) {\n        freeaddrinfo(result);\n        return 0;\n    }\n    \n    freeaddrinfo(result);\n    return 1;\n}\n\n/* Main DNS server function\n   Security: validates IP and port, implements bounds checking throughout */\nint startDNSServer(const char* listenIP, uint16_t port) {\n    /* Validate inputs */\n    if (listenIP == NULL) {\n        fprintf(stderr, "Invalid IP address (NULL)\\n");\n        return 0;\n    }\n    \n    /* Validate IP address format */\n    struct in_addr addr;\n    if (inet_pton(AF_INET, listenIP, &addr) != 1) {\n        fprintf(stderr, "Invalid IP address format\\n");\n        return 0;\n    }\n    \n    /* Validate port range */\n    if (port == 0) {\n        fprintf(stderr, "Invalid port number\\n");\n        return 0;\n    }\n    \n    /* Create UDP socket with error checking */\n    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);\n    if (sockfd < 0) {\n        fprintf(stderr, "Failed to create socket: %s\\n", strerror(errno));\n        return 0;\n    }\n    \n    struct sockaddr_in serverAddr;\n    memset(&serverAddr, 0, sizeof(serverAddr));\n    serverAddr.sin_family = AF_INET;\n    serverAddr.sin_port = htons(port);\n    serverAddr.sin_addr = addr;\n    \n    /* Bind socket with error checking */\n    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {\n        fprintf(stderr, "Failed to bind socket: %s\\n", strerror(errno));\n        close(sockfd);\n        return 0;\n    }\n    \n    printf("DNS server listening on %s:%u\\n", listenIP, port);\n    \n    /* Server loop with bounds-checked buffer */\n    uint8_t buffer[MAX_DNS_PACKET_SIZE];\n    uint8_t response[MAX_DNS_PACKET_SIZE];\n    struct sockaddr_in clientAddr;\n    socklen_t clientLen;\n    \n    while (1) {\n        /* Initialize buffers to zero */\n        memset(buffer, 0, sizeof(buffer));\n        memset(response, 0, sizeof(response));\n        memset(&clientAddr, 0, sizeof(clientAddr));\n        clientLen = sizeof(clientAddr);\n        \n        /* Receive with size limit to prevent overflow (CWE-120) */\n        ssize_t recvLen = recvfrom(sockfd, buffer, sizeof(buffer), 0,\n                                   (struct sockaddr*)&clientAddr, &clientLen);\n        \n        if (recvLen < 0) {\n            fprintf(stderr, "Receive error: %s\\n", strerror(errno));\n            continue;\n        }\n        \n        /* Validate minimum packet size */\n        if (recvLen < (ssize_t)sizeof(DNSHeader)) {\n            fprintf(stderr, "Packet too small\\n");\n            continue;\n        }\n        \n        /* Validate maximum packet size */\n        if ((size_t)recvLen > MAX_DNS_PACKET_SIZE) {\n            fprintf(stderr, "Packet too large\\n");\n            continue;\n        }\n        \n        /* Parse DNS header with bounds checking */\n        const DNSHeader* header = (const DNSHeader*)buffer;\n        uint16_t qdcount = ntohs(header->qdcount);\n        \n        /* Only handle single question queries */\n        if (qdcount != 1) {\n            fprintf(stderr, "Invalid question count\\n");\n            continue;\n        }\n        \n        /* Parse domain name with bounds checking */\n        char domain[MAX_DOMAIN_LENGTH + 1];\n        int nameLen = parseDomainName(buffer, (size_t)recvLen,\n                                     sizeof(DNSHeader), domain,\n                                     sizeof(domain));\n        \n        if (nameLen < 0 || domain[0] == '\\0') {\n            fprintf(stderr, "Failed to parse domain name\\n");\n            continue;\n        }\n        \n        printf("Received query for: %s\\n", domain);\n        \n        /* Resolve domain using system resolver */\n        char resolvedIP[INET_ADDRSTRLEN];\n        if (!resolveDomain(domain, resolvedIP, sizeof(resolvedIP))) {\n            printf("Failed to resolve domain: %s\\n", domain);\n            continue;\n        }\n        \n        printf("Resolved %s to %s\\n", domain, resolvedIP);\n        \n        /* Build response with bounds checking */\n        size_t responseLen = 0;\n        if (!buildDNSResponse(buffer, (size_t)recvLen, resolvedIP,\n                             response, sizeof(response), &responseLen)) {\n            fprintf(stderr, "Failed to build response\\n");\n            continue;\n        }\n        \n        /* Send response with error checking */\n        ssize_t sentLen = sendto(sockfd, response, responseLen, 0,\n                                (struct sockaddr*)&clientAddr, clientLen);\n        \n        if (sentLen < 0 || (size_t)sentLen != responseLen) {\n            fprintf(stderr, "Failed to send response: %s\\n", strerror(errno));\n            continue;\n        }\n        \n        printf("Response sent successfully\\n");\n    }\n    \n    close(sockfd);\n    return 1;\n}\n\nint main(void) {\n    /* Test case 1: Standard DNS server setup validation */\n    printf("Test 1: Validating DNS server parameters\\n");\n    \n    /* Test case 2: Validate IP address parsing */\n    printf("\\nTest 2: Validating IP address formats\\n");\n    struct in_addr testAddr;\n    printf("Valid IP (192.168.1.1): %s\\n",\n           inet_pton(AF_INET, "192.168.1.1", &testAddr) == 1 ? "PASS" : "FAIL");\n    printf("Invalid IP (999.999.999.999): %s\\n",\n           inet_pton(AF_INET, "999.999.999.999", &testAddr) != 1 ? "PASS" : "FAIL");\n    \n    /* Test case 3: Domain name parsing validation */\n    printf("\\nTest 3: Testing domain name parsing\\n");\n    uint8_t testQuery[] = {0x03, 'w', 'w', 'w', 0x06, 'g', 'o', 'o', 'g', 'l', 'e',\n                          0x03, 'c', 'o', 'm', 0x00};\n    char parsedDomain[MAX_DOMAIN_LENGTH + 1];\n    int result = parseDomainName(testQuery, sizeof(testQuery), 0,\n                                parsedDomain, sizeof(parsedDomain));\n    printf("Parsed domain: %s (%s)\\n",\n           (result > 0 && strcmp(parsedDomain, "www.google.com") == 0) ? "PASS" : "FAIL",\n           parsedDomain);\n    \n    /* Test case 4: Test domain resolution */\n    printf("\\nTest 4: Testing domain resolution\\n");\n    char resolved[INET_ADDRSTRLEN];\n    int resolveResult = resolveDomain("localhost", resolved, sizeof(resolved));\n    printf("Resolved localhost: %s (%s)\\n",\n           resolveResult ? "PASS" : "FAIL", resolved);\n    \n    /* Test case 5: Invalid domain handling */\n    printf("\\nTest 5: Testing invalid domain handling\\n");\n    char invalidDomain[400];\n    memset(invalidDomain, 'a', sizeof(invalidDomain) - 1);\n    invalidDomain[sizeof(invalidDomain) - 1] = '\\0';
    char invalidResolved[INET_ADDRSTRLEN];
    int invalidResult = resolveDomain(invalidDomain, invalidResolved,
                                     sizeof(invalidResolved));
    printf("Invalid domain rejected: %s\\n", !invalidResult ? "PASS" : "FAIL");
    
    printf("\\nAll validation tests completed.\\n");
    printf("To run the actual DNS server, uncomment the following line:\\n");
    printf("// startDNSServer(\\"127.0.0.1\\", 5353);\\n");
    
    return 0;
}
