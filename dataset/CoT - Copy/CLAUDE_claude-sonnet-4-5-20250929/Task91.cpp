
#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <thread>
#include <chrono>

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

class Task91 {
private:
    static const int DNS_HEADER_SIZE = 12;
    static const int MAX_BUFFER_SIZE = 512;
    std::map<std::string, std::string> dnsRecords;
    
public:
    Task91() {
        // Initialize DNS records
        dnsRecords["example.com"] = "93.184.216.34";
        dnsRecords["test.com"] = "192.0.2.1";
        dnsRecords["localhost"] = "127.0.0.1";
        dnsRecords["google.com"] = "172.217.164.46";
        dnsRecords["github.com"] = "140.82.121.4";
    }
    
    void startDNSServer(const std::string& ipAddress, int port) {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return;
        }
        #endif
        
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            #ifdef _WIN32
            WSACleanup();
            #endif
            return;
        }
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);
        
        if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(sock);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return;
        }
        
        std::cout << "DNS Server started on " << ipAddress << ":" << port << std::endl;
        
        char buffer[MAX_BUFFER_SIZE];
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // Set timeout
        struct timeval timeout;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        
        while (true) {
            memset(buffer, 0, MAX_BUFFER_SIZE);
            int recvLen = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0,
                                   (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (recvLen == SOCKET_ERROR) {
                std::cout << "Socket timeout or error, stopping server..." << std::endl;
                break;
            }
            
            if (recvLen < DNS_HEADER_SIZE) {
                continue;
            }
            
            std::vector<unsigned char> requestData(buffer, buffer + recvLen);
            std::vector<unsigned char> response = processDNSRequest(requestData);
            
            if (!response.empty()) {
                sendto(sock, (const char*)response.data(), response.size(), 0,
                       (struct sockaddr*)&clientAddr, clientAddrLen);
                
                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                std::cout << "Response sent to " << clientIP << ":" << ntohs(clientAddr.sin_port) << std::endl;
            }
        }
        
        closesocket(sock);
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    
    std::vector<unsigned char> processDNSRequest(const std::vector<unsigned char>& requestData) {
        if (requestData.size() < DNS_HEADER_SIZE) {
            return std::vector<unsigned char>();
        }
        
        try {
            // Parse DNS header
            unsigned short transactionId = (requestData[0] << 8) | requestData[1];
            unsigned short questions = (requestData[4] << 8) | requestData[5];
            
            if (questions <= 0 || questions > 10) {
                return std::vector<unsigned char>();
            }
            
            // Parse question section
            size_t offset = DNS_HEADER_SIZE;
            std::string domain = parseDomainName(requestData, offset);
            
            if (domain.empty() || domain.length() > 253) {
                return std::vector<unsigned char>();
            }
            
            if (offset + 4 > requestData.size()) {
                return std::vector<unsigned char>();
            }
            
            unsigned short qType = (requestData[offset] << 8) | requestData[offset + 1];
            unsigned short qClass = (requestData[offset + 2] << 8) | requestData[offset + 3];
            
            std::cout << "Query for: " << domain << " (Type: " << qType << ")" << std::endl;
            
            // Create response
            return createDNSResponse(transactionId, domain, qType, qClass, requestData);
            
        } catch (...) {
            std::cerr << "Error parsing DNS request" << std::endl;
            return std::vector<unsigned char>();
        }
    }
    
    std::string parseDomainName(const std::vector<unsigned char>& data, size_t& offset) {
        std::string domain;
        size_t originalOffset = offset;
        int jumps = 0;
        const int maxJumps = 10;
        
        try {
            while (offset < data.size()) {
                unsigned char length = data[offset];
                
                if (length == 0) {
                    offset++;
                    break;
                }
                
                // Check for pointer (compression)
                if ((length & 0xC0) == 0xC0) {
                    if (offset + 1 >= data.size()) {
                        break;
                    }
                    size_t pointer = ((length & 0x3F) << 8) | data[offset + 1];
                    if (pointer >= data.size() || jumps >= maxJumps) {
                        break;
                    }
                    offset = pointer;
                    jumps++;
                    continue;
                }
                
                if (length > 63 || offset + length + 1 > data.size()) {
                    break;
                }
                
                if (!domain.empty()) {
                    domain += '.';
                }
                
                for (size_t i = 0; i < length; i++) {
                    char c = data[offset + 1 + i];
                    if (isalnum(c) || c == '-' || c == '.') {
                        domain += c;
                    }
                }
                
                offset += length + 1;
                
                if (domain.length() > 253) {
                    break;
                }
            }
            
            if (jumps == 0) {
                // Normal case, offset is correct
            } else {
                offset = originalOffset + 2;
            }
            
        } catch (...) {
            return "";
        }
        
        return domain;
    }
    
    std::vector<unsigned char> createDNSResponse(unsigned short transactionId,
                                                  const std::string& domain,
                                                  unsigned short qType,
                                                  unsigned short qClass,
                                                  const std::vector<unsigned char>& originalQuery) {
        try {
            std::vector<unsigned char> response;
            
            // DNS Header
            response.push_back(transactionId >> 8);
            response.push_back(transactionId & 0xFF);
            response.push_back(0x81);
            response.push_back(0x80);
            response.push_back(0x00);
            response.push_back(0x01); // Questions
            
            std::string lowerDomain = domain;
            for (char& c : lowerDomain) c = tolower(c);
            
            auto it = dnsRecords.find(lowerDomain);
            bool hasAnswer = (it != dnsRecords.end());
            
            response.push_back(0x00);
            response.push_back(hasAnswer ? 0x01 : 0x00); // Answer RRs
            response.push_back(0x00);
            response.push_back(0x00); // Authority RRs
            response.push_back(0x00);
            response.push_back(0x00); // Additional RRs
            
            // Question section (copy from original)
            size_t questionStart = DNS_HEADER_SIZE;
            size_t questionEnd = questionStart;
            
            while (questionEnd < originalQuery.size() && originalQuery[questionEnd] != 0) {
                questionEnd++;
            }
            questionEnd += 5; // Include null terminator, type and class
            
            if (questionEnd <= originalQuery.size()) {
                response.insert(response.end(),
                              originalQuery.begin() + questionStart,
                              originalQuery.begin() + questionEnd);
            }
            
            // Answer section
            if (hasAnswer) {
                response.push_back(0xC0);
                response.push_back(0x0C); // Pointer to domain name
                response.push_back(qType >> 8);
                response.push_back(qType & 0xFF);
                response.push_back(qClass >> 8);
                response.push_back(qClass & 0xFF);
                response.push_back(0x00);
                response.push_back(0x00);
                response.push_back(0x01);
                response.push_back(0x2C); // TTL (300 seconds)
                response.push_back(0x00);
                response.push_back(0x04); // Data length
                
                // IP address
                std::string ipAddress = it->second;
                size_t pos = 0;
                for (int i = 0; i < 4; i++) {
                    size_t dotPos = ipAddress.find('.', pos);
                    std::string part = (dotPos != std::string::npos) ?
                                     ipAddress.substr(pos, dotPos - pos) :
                                     ipAddress.substr(pos);
                    int value = std::stoi(part);
                    if (value >= 0 && value <= 255) {
                        response.push_back(static_cast<unsigned char>(value));
                    }
                    pos = dotPos + 1;
                }
            }
            
            return response;
            
        } catch (...) {
            std::cerr << "Error creating DNS response" << std::endl;
            return std::vector<unsigned char>();
        }
    }
    
    void sendDNSQuery(const std::string& serverIp, int serverPort, const std::string& domain) {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
        
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
        
        std::vector<unsigned char> query;
        
        // DNS Header
        query.push_back(0x12);
        query.push_back(0x34); // Transaction ID
        query.push_back(0x01);
        query.push_back(0x00); // Standard query
        query.push_back(0x00);
        query.push_back(0x01); // Questions
        query.push_back(0x00);
        query.push_back(0x00); // Answer RRs
        query.push_back(0x00);
        query.push_back(0x00); // Authority RRs
        query.push_back(0x00);
        query.push_back(0x00); // Additional RRs
        
        // Domain name
        size_t pos = 0;
        while (pos < domain.length()) {
            size_t dotPos = domain.find('.', pos);
            std::string label = (dotPos != std::string::npos) ?
                              domain.substr(pos, dotPos - pos) :
                              domain.substr(pos);
            
            query.push_back(static_cast<unsigned char>(label.length()));
            for (char c : label) {
                query.push_back(static_cast<unsigned char>(c));
            }
            
            if (dotPos == std::string::npos) break;
            pos = dotPos + 1;
        }
        query.push_back(0x00);
        
        query.push_back(0x00);
        query.push_back(0x01); // Type A
        query.push_back(0x00);
        query.push_back(0x01); // Class IN
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
        
        sendto(sock, (const char*)query.data(), query.size(), 0,
               (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        
        char buffer[MAX_BUFFER_SIZE];
        struct sockaddr_in fromAddr;
        socklen_t fromAddrLen = sizeof(fromAddr);
        
        int recvLen = recvfrom(sock, buffer, MAX_BUFFER_SIZE, 0,
                               (struct sockaddr*)&fromAddr, &fromAddrLen);
        
        if (recvLen > 0) {
            std::cout << "Received response for " << domain << " (" << recvLen << " bytes)" << std::endl;
        } else {
            std::cerr << "Query error for " << domain << std::endl;
        }
        
        closesocket(sock);
        
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
};

int main() {
    std::cout << "=== DNS Server Test Cases ===" << std::endl << std::endl;
    
    Task91 dnsServer;
    
    // Start server in separate thread
    std::thread serverThread([&dnsServer]() {
        dnsServer.startDNSServer("127.0.0.1", 5353);
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Test cases
    std::vector<std::string> testDomains = {
        "example.com", "test.com", "localhost", "google.com", "github.com"
    };
    
    for (size_t i = 0; i < testDomains.size(); i++) {
        std::cout << "\\nTest Case " << (i + 1) << ": Querying " << testDomains[i] << std::endl;
        dnsServer.sendDNSQuery("127.0.0.1", 5353, testDomains[i]);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}
