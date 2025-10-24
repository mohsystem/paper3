
#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdexcept>

// Maximum buffer size to prevent excessive memory usage
constexpr size_t MAX_DNS_PACKET_SIZE = 512;
constexpr size_t MAX_DOMAIN_LENGTH = 253;
constexpr uint16_t DNS_QUERY_TYPE_A = 1;
constexpr uint16_t DNS_CLASS_IN = 1;

// DNS Header structure
struct DNSHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

// Parse domain name from DNS query, returns length parsed or -1 on error
// Security: bounds checking to prevent buffer overflow (CWE-119, CWE-125)
int parseDomainName(const uint8_t* buffer, size_t bufferLen, size_t offset, 
                    std::string& domain, size_t maxLen = MAX_DOMAIN_LENGTH) {
    if (buffer == nullptr || bufferLen == 0) return -1;
    
    size_t pos = offset;
    size_t totalLen = 0;
    
    while (pos < bufferLen) {
        // Bounds check before reading length byte
        if (pos >= bufferLen) return -1;
        
        uint8_t len = buffer[pos];
        
        // Check for compression pointer (not supported in this simple implementation)
        if ((len & 0xC0) == 0xC0) {
            return -1; // Reject compression to prevent complexity attacks
        }
        
        if (len == 0) {
            pos++;
            break;
        }
        
        // Validate label length
        if (len > 63) return -1; // DNS label max is 63 bytes
        
        pos++;
        
        // Bounds check: ensure we don't read beyond buffer\n        if (pos + len > bufferLen) return -1;\n        \n        // Check total domain length limit\n        if (totalLen + len + 1 > maxLen) return -1;\n        \n        if (!domain.empty()) {\n            domain += '.';\n        }\n        \n        // Safe copy with bounds checking\n        for (size_t i = 0; i < len; i++) {\n            if (pos + i >= bufferLen) return -1;\n            char c = static_cast<char>(buffer[pos + i]);\n            // Validate DNS label characters (alphanumeric, hyphen)\n            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n                  (c >= '0' && c <= '9') || c == '-' || c == '_')) {\n                return -1;\n            }\n            domain += c;\n        }\n        \n        pos += len;\n        totalLen += len + 1;\n    }\n    \n    return static_cast<int>(pos - offset);\n}\n\n// Build DNS response with bounds checking\n// Security: prevent buffer overflow (CWE-787)\nbool buildDNSResponse(const uint8_t* query, size_t queryLen, \n                     const std::string& resolvedIP, \n                     std::vector<uint8_t>& response) {\n    if (query == nullptr || queryLen < sizeof(DNSHeader) + 6) {\n        return false;\n    }\n    \n    // Copy query header with bounds checking\n    if (queryLen > MAX_DNS_PACKET_SIZE) return false;\n    \n    response.assign(query, query + queryLen);\n    \n    // Modify header flags: set response bit and recursion available\n    if (response.size() < sizeof(DNSHeader)) return false;\n    \n    DNSHeader* header = reinterpret_cast<DNSHeader*>(response.data());\n    header->flags = htons(0x8180); // Standard response, no error\n    header->ancount = htons(1); // One answer\n    \n    // Add answer section: name pointer to question\n    response.push_back(0xC0); // Compression pointer\n    response.push_back(0x0C); // Offset to question name\n    \n    // Type A\n    response.push_back(0x00);\n    response.push_back(DNS_QUERY_TYPE_A);\n    \n    // Class IN\n    response.push_back(0x00);\n    response.push_back(DNS_CLASS_IN);\n    \n    // TTL (300 seconds)\n    response.push_back(0x00);\n    response.push_back(0x00);\n    response.push_back(0x01);\n    response.push_back(0x2C);\n    \n    // Data length (4 bytes for IPv4)\n    response.push_back(0x00);\n    response.push_back(0x04);\n    \n    // Parse and add IP address with validation\n    struct in_addr addr;\n    if (inet_pton(AF_INET, resolvedIP.c_str(), &addr) != 1) {\n        return false;\n    }\n    \n    uint32_t ip = ntohl(addr.s_addr);\n    response.push_back((ip >> 24) & 0xFF);\n    response.push_back((ip >> 16) & 0xFF);\n    response.push_back((ip >> 8) & 0xFF);\n    response.push_back(ip & 0xFF);\n    \n    // Final size check\n    if (response.size() > MAX_DNS_PACKET_SIZE) {\n        return false;\n    }\n    \n    return true;\n}\n\n// Resolve DNS query using getaddrinfo (system resolver)\n// Security: uses system resolver, validates input domain length\nstd::string resolveDomain(const std::string& domain) {\n    if (domain.empty() || domain.length() > MAX_DOMAIN_LENGTH) {\n        return "";\n    }\n    \n    struct addrinfo hints, *result = nullptr;\n    std::memset(&hints, 0, sizeof(hints));\n    hints.ai_family = AF_INET;\n    hints.ai_socktype = SOCK_DGRAM;\n    \n    // Use getaddrinfo with validated domain name\n    int ret = getaddrinfo(domain.c_str(), nullptr, &hints, &result);\n    if (ret != 0 || result == nullptr) {\n        if (result) freeaddrinfo(result);\n        return "";\n    }\n    \n    char ipstr[INET_ADDRSTRLEN];\n    std::memset(ipstr, 0, sizeof(ipstr));\n    \n    struct sockaddr_in* addr = reinterpret_cast<struct sockaddr_in*>(result->ai_addr);\n    if (inet_ntop(AF_INET, &(addr->sin_addr), ipstr, sizeof(ipstr)) == nullptr) {\n        freeaddrinfo(result);\n        return "";\n    }\n    \n    std::string resolvedIP(ipstr);\n    freeaddrinfo(result);\n    \n    return resolvedIP;\n}\n\n// Main DNS server function\n// Security: validates IP and port, implements bounds checking throughout\nbool startDNSServer(const std::string& listenIP, uint16_t port) {\n    // Validate IP address format\n    struct in_addr addr;\n    if (inet_pton(AF_INET, listenIP.c_str(), &addr) != 1) {\n        std::cerr << "Invalid IP address format" << std::endl;\n        return false;\n    }\n    \n    // Validate port range (1-65535, avoid privileged ports < 1024 in production)\n    if (port == 0) {\n        std::cerr << "Invalid port number" << std::endl;\n        return false;\n    }\n    \n    // Create UDP socket with error checking\n    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);\n    if (sockfd < 0) {\n        std::cerr << "Failed to create socket" << std::endl;\n        return false;\n    }\n    \n    struct sockaddr_in serverAddr;\n    std::memset(&serverAddr, 0, sizeof(serverAddr));\n    serverAddr.sin_family = AF_INET;\n    serverAddr.sin_port = htons(port);\n    serverAddr.sin_addr = addr;\n    \n    // Bind socket with error checking\n    if (bind(sockfd, reinterpret_cast<struct sockaddr*>(&serverAddr), \n             sizeof(serverAddr)) < 0) {\n        std::cerr << "Failed to bind socket" << std::endl;\n        close(sockfd);\n        return false;\n    }\n    \n    std::cout << "DNS server listening on " << listenIP << ":" << port << std::endl;\n    \n    // Server loop with bounds-checked buffer\n    uint8_t buffer[MAX_DNS_PACKET_SIZE];\n    struct sockaddr_in clientAddr;\n    socklen_t clientLen = sizeof(clientAddr);\n    \n    while (true) {\n        std::memset(buffer, 0, sizeof(buffer));\n        std::memset(&clientAddr, 0, sizeof(clientAddr));\n        clientLen = sizeof(clientAddr);\n        \n        // Receive with size limit to prevent overflow (CWE-120)\n        ssize_t recvLen = recvfrom(sockfd, buffer, sizeof(buffer), 0,\n                                   reinterpret_cast<struct sockaddr*>(&clientAddr),\n                                   &clientLen);\n        \n        if (recvLen < 0) {\n            std::cerr << "Receive error" << std::endl;\n            continue;\n        }\n        \n        if (recvLen < static_cast<ssize_t>(sizeof(DNSHeader))) {\n            std::cerr << "Packet too small" << std::endl;\n            continue;\n        }\n        \n        // Validate packet size\n        if (static_cast<size_t>(recvLen) > MAX_DNS_PACKET_SIZE) {\n            std::cerr << "Packet too large" << std::endl;\n            continue;\n        }\n        \n        // Parse DNS header with bounds checking\n        const DNSHeader* header = reinterpret_cast<const DNSHeader*>(buffer);\n        uint16_t qdcount = ntohs(header->qdcount);\n        \n        // Only handle single question queries\n        if (qdcount != 1) {\n            std::cerr << "Invalid question count" << std::endl;\n            continue;\n        }\n        \n        // Parse domain name with bounds checking\n        std::string domain;\n        int nameLen = parseDomainName(buffer, static_cast<size_t>(recvLen), \n                                     sizeof(DNSHeader), domain);\n        \n        if (nameLen < 0 || domain.empty()) {\n            std::cerr << "Failed to parse domain name" << std::endl;\n            continue;\n        }\n        \n        std::cout << "Received query for: " << domain << std::endl;\n        \n        // Resolve domain using system resolver\n        std::string resolvedIP = resolveDomain(domain);\n        \n        if (resolvedIP.empty()) {\n            std::cout << "Failed to resolve domain: " << domain << std::endl;\n            continue;\n        }\n        \n        std::cout << "Resolved " << domain << " to " << resolvedIP << std::endl;\n        \n        // Build response with bounds checking\n        std::vector<uint8_t> response;\n        if (!buildDNSResponse(buffer, static_cast<size_t>(recvLen), \n                             resolvedIP, response)) {\n            std::cerr << "Failed to build response" << std::endl;\n            continue;\n        }\n        \n        // Send response with error checking\n        ssize_t sentLen = sendto(sockfd, response.data(), response.size(), 0,\n                                reinterpret_cast<struct sockaddr*>(&clientAddr),\n                                clientLen);\n        \n        if (sentLen < 0 || static_cast<size_t>(sentLen) != response.size()) {\n            std::cerr << "Failed to send response" << std::endl;\n            continue;\n        }\n        \n        std::cout << "Response sent successfully" << std::endl;\n    }\n    \n    close(sockfd);\n    return true;\n}\n\nint main() {\n    // Test case 1: Standard DNS server on localhost\n    std::cout << "Test 1: Starting DNS server on 127.0.0.1:5353" << std::endl;\n    // Note: In production, this would run indefinitely\n    // For testing, we demonstrate the setup only\n    \n    // Test case 2: Validate IP address parsing\n    std::cout << "\\nTest 2: Validating IP address formats" << std::endl;\n    struct in_addr testAddr;\n    std::cout << "Valid IP (192.168.1.1): " \n              << (inet_pton(AF_INET, "192.168.1.1", &testAddr) == 1 ? "PASS" : "FAIL") \n              << std::endl;\n    std::cout << "Invalid IP (999.999.999.999): " \n              << (inet_pton(AF_INET, "999.999.999.999", &testAddr) != 1 ? "PASS" : "FAIL") \n              << std::endl;\n    \n    // Test case 3: Domain name parsing validation\n    std::cout << "\\nTest 3: Testing domain name parsing" << std::endl;\n    uint8_t testQuery[] = {0x03, 'w', 'w', 'w', 0x06, 'g', 'o', 'o', 'g', 'l', 'e', 0x03, 'c', 'o', 'm', 0x00};\n    std::string parsedDomain;\n    int result = parseDomainName(testQuery, sizeof(testQuery), 0, parsedDomain);\n    std::cout << "Parsed domain: " << (parsedDomain == "www.google.com" ? "PASS" : "FAIL") \n              << " (" << parsedDomain << ")" << std::endl;\n    \n    // Test case 4: Test domain resolution\n    std::cout << "\\nTest 4: Testing domain resolution" << std::endl;\n    std::string resolved = resolveDomain("localhost");\n    std::cout << "Resolved localhost: " << (!resolved.empty() ? "PASS" : "FAIL") \n              << " (" << resolved << ")" << std::endl;\n    \n    // Test case 5: Invalid domain handling\n    std::cout << "\\nTest 5: Testing invalid domain handling" << std::endl;\n    std::string invalidDomain(300, 'a'); // Exceeds MAX_DOMAIN_LENGTH
    std::string invalidResolved = resolveDomain(invalidDomain);
    std::cout << "Invalid domain rejected: " 
              << (invalidResolved.empty() ? "PASS" : "FAIL") << std::endl;
    
    std::cout << "\\nAll validation tests completed." << std::endl;
    std::cout << "To run the actual DNS server, uncomment the following line:" << std::endl;
    std::cout << "// startDNSServer(\\"127.0.0.1\\", 5353);" << std::endl;
    
    return 0;
}
