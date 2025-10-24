
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>

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

class Task91 {
private:
    static std::map<std::string, std::string> dnsRecords;
    
    static void initDNSRecords() {
        dnsRecords["example.com"] = "93.184.216.34";
        dnsRecords["test.local"] = "127.0.0.1";
        dnsRecords["localhost"] = "127.0.0.1";
        dnsRecords["google.com"] = "142.250.185.78";
        dnsRecords["github.com"] = "140.82.121.3";
    }
    
public:
    static void startDNSServer(const std::string& ipAddress, int port) {
        initDNSRecords();
        
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);
        
        if (bind(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(sock);
            return;
        }
        
        std::cout << "DNS Server listening on " << ipAddress << ":" << port << std::endl;
        
        char buffer[512];
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        while (true) {
            int recvLen = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientLen);
            if (recvLen > 0) {
                std::vector<unsigned char> response = processDNSRequest((unsigned char*)buffer, recvLen);
                sendto(sock, (char*)response.data(), response.size(), 0, (sockaddr*)&clientAddr, clientLen);
                
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
    
    static std::vector<unsigned char> processDNSRequest(unsigned char* data, int length) {
        try {
            unsigned short transactionId = (data[0] << 8) | data[1];
            
            int offset = 12;
            std::string domainName = parseDomainName(data, offset);
            unsigned short queryType = (data[offset] << 8) | data[offset + 1];
            unsigned short queryClass = (data[offset + 2] << 8) | data[offset + 3];
            
            std::cout << "Query for: " << domainName << std::endl;
            
            return buildDNSResponse(transactionId, domainName, queryType, queryClass);
        } catch (...) {
            return std::vector<unsigned char>();
        }
    }
    
    static std::string parseDomainName(unsigned char* data, int& offset) {
        std::string domain;
        int length;
        
        while ((length = data[offset++]) != 0) {
            if (!domain.empty()) domain += '.';
            for (int i = 0; i < length; i++) {
                domain += (char)data[offset++];
            }
        }
        
        return domain;
    }
    
    static std::vector<unsigned char> buildDNSResponse(unsigned short transactionId, const std::string& domain, 
                                                       unsigned short queryType, unsigned short queryClass) {
        std::vector<unsigned char> response;
        
        response.push_back(transactionId >> 8);
        response.push_back(transactionId & 0xFF);
        response.push_back(0x81);
        response.push_back(0x80);
        response.push_back(0x00);
        response.push_back(0x01);
        response.push_back(0x00);
        response.push_back(0x01);
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x00);
        
        encodeDomainName(response, domain);
        response.push_back(queryType >> 8);
        response.push_back(queryType & 0xFF);
        response.push_back(queryClass >> 8);
        response.push_back(queryClass & 0xFF);
        
        encodeDomainName(response, domain);
        response.push_back(queryType >> 8);
        response.push_back(queryType & 0xFF);
        response.push_back(queryClass >> 8);
        response.push_back(queryClass & 0xFF);
        response.push_back(0x00);
        response.push_back(0x00);
        response.push_back(0x01);
        response.push_back(0x2C);
        
        std::string ipAddress = dnsRecords.count(domain) ? dnsRecords[domain] : "0.0.0.0";
        response.push_back(0x00);
        response.push_back(0x04);
        
        size_t start = 0, end;
        while ((end = ipAddress.find('.', start)) != std::string::npos) {
            response.push_back(std::stoi(ipAddress.substr(start, end - start)));
            start = end + 1;
        }
        response.push_back(std::stoi(ipAddress.substr(start)));
        
        return response;
    }
    
    static void encodeDomainName(std::vector<unsigned char>& buffer, const std::string& domain) {
        size_t start = 0, end;
        while ((end = domain.find('.', start)) != std::string::npos) {
            std::string label = domain.substr(start, end - start);
            buffer.push_back(label.length());
            for (char c : label) buffer.push_back(c);
            start = end + 1;
        }
        std::string label = domain.substr(start);
        buffer.push_back(label.length());
        for (char c : label) buffer.push_back(c);
        buffer.push_back(0);
    }
    
    static void testDNSQuery(const std::string& domain, int port) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
        
        std::vector<unsigned char> query = buildDNSQuery(domain);
        
        std::cout << "Test: Querying " << domain << std::endl;
        sendto(sock, (char*)query.data(), query.size(), 0, (sockaddr*)&serverAddr, sizeof(serverAddr));
        
        char buffer[512];
        int recvLen = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        
        if (recvLen > 0) {
            std::cout << "Success: Received response for " << domain << "\\n" << std::endl;
        }
        
        closesocket(sock);
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    static std::vector<unsigned char> buildDNSQuery(const std::string& domain) {
        std::vector<unsigned char> query;
        query.push_back(0x12);
        query.push_back(0x34);
        query.push_back(0x01);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x01);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x00);
        query.push_back(0x00);
        
        encodeDomainName(query, domain);
        query.push_back(0x00);
        query.push_back(0x01);
        query.push_back(0x00);
        query.push_back(0x01);
        
        return query;
    }
};

std::map<std::string, std::string> Task91::dnsRecords;

int main() {
    const int DNS_PORT = 5353;
    std::cout << "=== DNS Server Test Cases ===\\n" << std::endl;
    
    std::thread serverThread([]() {
        Task91::startDNSServer("127.0.0.1", 5353);
    });
    serverThread.detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::vector<std::string> testDomains = {"example.com", "test.local", "localhost", "google.com", "github.com"};
    
    for (const auto& domain : testDomains) {
        Task91::testDNSQuery(domain, DNS_PORT);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
