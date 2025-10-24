
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <cstring>
#include <sstream>

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
    static std::map<std::string, std::string> DNS_RECORDS;
    
public:
    static void initializeDNSRecords() {
        DNS_RECORDS["example.com"] = "93.184.216.34";
        DNS_RECORDS["test.com"] = "192.168.1.100";
        DNS_RECORDS["localhost"] = "127.0.0.1";
        DNS_RECORDS["google.com"] = "142.250.185.46";
        DNS_RECORDS["github.com"] = "140.82.114.4";
    }
    
    static void startDNSServer(const std::string& ipAddress, int port) {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
        
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Failed to create socket" << std::endl;
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
            return;
        }
        
        std::cout << "DNS Server listening on " << ipAddress << ":" << port << std::endl;
        
        unsigned char buffer[512];
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int recvLen = recvfrom(sock, (char*)buffer, 512, 0, 
                               (struct sockaddr*)&clientAddr, &clientLen);
        
        if (recvLen > 0) {
            std::vector<unsigned char> query(buffer, buffer + recvLen);
            std::string domainName = parseDNSQuery(query);
            std::cout << "Received query for: " << domainName << std::endl;
            
            std::string ipAddr = resolveDNSRecord(domainName);
            std::vector<unsigned char> response = createDNSResponse(query, ipAddr);
            
            sendto(sock, (char*)response.data(), response.size(), 0,
                   (struct sockaddr*)&clientAddr, clientLen);
            std::cout << "Sent response: " << ipAddr << std::endl;
        }
        
        closesocket(sock);
        
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    
    static std::string parseDNSQuery(const std::vector<unsigned char>& query) {
        std::string domain;
        size_t pos = 12; // Skip DNS header
        
        while (pos < query.size() && query[pos] != 0) {
            int length = query[pos];
            if (length == 0) break;
            
            if (!domain.empty()) domain += ".";
            pos++;
            
            for (int i = 0; i < length && pos < query.size(); i++, pos++) {
                domain += (char)query[pos];
            }
        }
        
        return domain;
    }
    
    static std::string resolveDNSRecord(const std::string& domainName) {
        auto it = DNS_RECORDS.find(domainName);
        return (it != DNS_RECORDS.end()) ? it->second : "0.0.0.0";
    }
    
    static std::vector<unsigned char> createDNSResponse(
        const std::vector<unsigned char>& query, const std::string& ipAddress) {
        
        std::vector<unsigned char> response;
        
        // Copy transaction ID
        response.push_back(query[0]);
        response.push_back(query[1]);
        
        // Flags: Response, Standard Query, No Error
        response.push_back(0x81);
        response.push_back(0x80);
        
        // Questions count
        response.push_back(query[4]);
        response.push_back(query[5]);
        
        // Answers count
        response.push_back(0x00);
        response.push_back(0x01);
        
        // Authority and Additional RRs
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x00);
        
        // Copy question section
        size_t pos = 12;
        while (pos < query.size() && query[pos] != 0) {
            response.push_back(query[pos++]);
        }
        response.push_back(0x00);
        pos++;
        
        // Copy QTYPE and QCLASS
        for (int i = 0; i < 4 && pos < query.size(); i++, pos++) {
            response.push_back(query[pos]);
        }
        
        // Answer section
        response.push_back(0xC0);
        response.push_back(0x0C);
        
        // Type A
        response.push_back(0x00);
        response.push_back(0x01);
        
        // Class IN
        response.push_back(0x00);
        response.push_back(0x01);
        
        // TTL (300 seconds)
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x01);
        response.push_back(0x2C);
        
        // Data length
        response.push_back(0x00);
        response.push_back(0x04);
        
        // IP Address
        std::istringstream iss(ipAddress);
        std::string octet;
        while (std::getline(iss, octet, '.')) {
            response.push_back((unsigned char)std::stoi(octet));
        }
        
        return response;
    }
    
    static std::vector<unsigned char> createTestQuery(const std::string& domain) {
        std::vector<unsigned char> query;
        query.push_back(0x12);
        query.push_back(0x34);
        query.push_back(0x01);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x01);
        for (int i = 0; i < 6; i++) query.push_back(0x00);
        
        std::istringstream iss(domain);
        std::string label;
        while (std::getline(iss, label, '.')) {
            query.push_back((unsigned char)label.length());
            for (char c : label) query.push_back((unsigned char)c);
        }
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x01);
        query.push_back(0x00);
        query.push_back(0x01);
        
        return query;
    }
};

std::map<std::string, std::string> Task91::DNS_RECORDS;

int main() {
    Task91::initializeDNSRecords();
    
    std::cout << "DNS Server Test Cases:" << std::endl;
    std::cout << "======================" << std::endl << std::endl;
    
    // Test Case 1: Parse DNS query
    std::cout << "Test 1: Parse DNS Query" << std::endl;
    std::vector<unsigned char> testQuery1 = Task91::createTestQuery("example.com");
    std::string parsed1 = Task91::parseDNSQuery(testQuery1);
    std::cout << "Parsed domain: " << parsed1 << std::endl << std::endl;
    
    // Test Case 2: Resolve DNS record
    std::cout << "Test 2: Resolve DNS Record" << std::endl;
    std::string resolved = Task91::resolveDNSRecord("google.com");
    std::cout << "Resolved IP: " << resolved << std::endl << std::endl;
    
    // Test Case 3: Resolve unknown domain
    std::cout << "Test 3: Resolve Unknown Domain" << std::endl;
    std::string unknownResolved = Task91::resolveDNSRecord("unknown.com");
    std::cout << "Resolved IP: " << unknownResolved << std::endl << std::endl;
    
    // Test Case 4: Create DNS response
    std::cout << "Test 4: Create DNS Response" << std::endl;
    std::vector<unsigned char> response = Task91::createDNSResponse(testQuery1, "93.184.216.34");
    std::cout << "Response created with length: " << response.size() << std::endl << std::endl;
    
    // Test Case 5: Multiple domain parsing
    std::cout << "Test 5: Parse Multiple Domains" << std::endl;
    std::string domains[] = {"localhost", "test.com", "github.com"};
    for (const std::string& domain : domains) {
        std::vector<unsigned char> query = Task91::createTestQuery(domain);
        std::string parsed = Task91::parseDNSQuery(query);
        std::string ip = Task91::resolveDNSRecord(parsed);
        std::cout << domain << " -> " << parsed << " -> " << ip << std::endl;
    }
    
    return 0;
}
