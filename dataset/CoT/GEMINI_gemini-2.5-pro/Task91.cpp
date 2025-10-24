#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <stdexcept>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int SOCKET;
    const int INVALID_SOCKET = -1;
    const int SOCKET_ERROR = -1;
    #define closesocket close
#endif

class Task91 {
private:
    static const int PORT = 5353;
    static const int BUFFER_SIZE = 512;
    static std::map<std::string, std::string> dnsRecords;

    static std::string parseDomainName(const unsigned char* buffer, int& pos, int buffer_len) {
        std::string name = "";
        int current_pos = pos;
        
        while (current_pos < buffer_len && buffer[current_pos] != 0) {
            unsigned char len = buffer[current_pos];
            if (len == 0) break;
            
            // Check for name compression
            if ((len & 0xC0) == 0xC0) {
                 if (current_pos + 1 >= buffer_len) throw std::runtime_error("Malformed packet: pointer out of bounds");
                int pointer = ((len & 0x3F) << 8) + buffer[current_pos + 1];
                int dummy_pos = pointer;
                name += parseDomainName(buffer, dummy_pos, buffer_len);
                pos = current_pos + 2; // Move original position past the 2-byte pointer
                return name;
            }

            if (current_pos + 1 + len > buffer_len) throw std::runtime_error("Malformed packet: label out of bounds");
            
            name.append((const char*)&buffer[current_pos + 1], len);
            name.append(".");
            current_pos += (len + 1);
        }
        
        pos = (buffer[current_pos] == 0) ? current_pos + 1 : current_pos;
        return name;
    }

    static std::vector<unsigned char> buildDnsResponse(const unsigned char* request, int requestLen) {
        if (requestLen < 12) {
            std::cerr << "Request too short." << std::endl;
            return {};
        }

        // --- Parse request ---
        int pos = 12;
        std::string domainName;
        try {
            domainName = parseDomainName(request, pos, requestLen);
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
            return {};
        }
        
        if (pos + 4 > requestLen) {
             std::cerr << "Request missing QTYPE/QCLASS." << std::endl;
             return {};
        }

        uint16_t qtype = (request[pos] << 8) | request[pos+1];
        uint16_t qclass = (request[pos+2] << 8) | request[pos+3];

        if (qtype != 1 || qclass != 1 || dnsRecords.find(domainName) == dnsRecords.end()) {
            std::cout << "Unsupported query or unknown domain: " << domainName << std::endl;
            return {};
        }
        std::cout << "Query for: " << domainName << std::endl;
        
        // --- Build response ---
        std::vector<unsigned char> response;
        response.insert(response.end(), request, request + 12); // Copy header

        // Set response flags (QR=1, Opcode=0, AA=1, RCODE=0 -> 0x8400)
        response[2] = 0x84;
        response[3] = 0x00;

        // Set ANCOUNT to 1
        response[6] = 0x00;
        response[7] = 0x01;

        // Copy question section
        int questionLen = (pos + 4) - 12;
        response.insert(response.end(), request + 12, request + 12 + questionLen);

        // Add Answer Section
        // Name pointer (to offset 12)
        response.push_back(0xC0);
        response.push_back(0x0C);
        
        // Type (A), Class (IN)
        response.push_back(0x00); response.push_back(0x01);
        response.push_back(0x00); response.push_back(0x01);

        // TTL (60s)
        uint32_t ttl = htonl(60);
        unsigned char* ttl_bytes = (unsigned char*)&ttl;
        response.insert(response.end(), ttl_bytes, ttl_bytes + 4);

        // RDLENGTH (4)
        response.push_back(0x00); response.push_back(0x04);
        
        // RDATA (IP address)
        in_addr addr;
        inet_pton(AF_INET, dnsRecords[domainName].c_str(), &addr);
        unsigned char* ip_bytes = (unsigned char*)&addr.s_addr;
        response.insert(response.end(), ip_bytes, ip_bytes + 4);

        return response;
    }

public:
    static void startDnsServer(const char* ip, int port) {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return;
        }
        #endif

        SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == INVALID_SOCKET) {
            std::cerr << "Socket creation failed." << std::endl;
            return;
        }

        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &server_addr.sin_addr);

        if (bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed." << std::endl;
            closesocket(sock);
            return;
        }

        std::cout << "DNS server listening on " << ip << ":" << port << std::endl;

        unsigned char buffer[BUFFER_SIZE];
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        while (true) {
            int recv_len = recvfrom(sock, (char*)buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
            if (recv_len == SOCKET_ERROR) {
                std::cerr << "recvfrom failed." << std::endl;
                continue;
            }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "Received request from " << client_ip << std::endl;

            std::vector<unsigned char> response = buildDnsResponse(buffer, recv_len);
            if (!response.empty()) {
                sendto(sock, (const char*)response.data(), response.size(), 0, (struct sockaddr*)&client_addr, client_len);
            }
        }
        
        closesocket(sock);
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
};

std::map<std::string, std::string> Task91::dnsRecords = {
    {"test1.local.", "192.168.1.1"},
    {"test2.local.", "192.168.1.2"},
    {"example.com.", "93.184.216.34"},
    {"my-server.net.", "10.0.0.1"},
    {"localhost.", "127.0.0.1"}
};

int main() {
    std::cout << "Starting C++ DNS Server..." << std::endl;
    std::cout << "You can test this server with commands like:" << std::endl;
    std::cout << "  dig @127.0.0.1 -p 5353 test1.local" << std::endl;
    std::cout << "  nslookup example.com 127.0.0.1" << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
    Task91::startDnsServer("127.0.0.1", 5353);
    return 0;
}