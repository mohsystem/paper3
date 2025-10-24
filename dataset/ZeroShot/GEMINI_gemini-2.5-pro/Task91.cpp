#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cstring>
#include <stdexcept>

// Platform-specific includes for networking
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

// Constants
const int PORT = 53530;
const char* IP_ADDRESS = "127.0.0.1";
const int BUFFER_SIZE = 512;

// DNS records map
std::unordered_map<std::string, std::string> dnsRecords = {
    {"test1.example.com.", "192.0.2.1"},
    {"test2.example.com.", "198.51.100.2"},
    {"secure.example.com.", "203.0.113.3"}
};

// DNS Header structure. Use packed attribute for correct memory layout.
#pragma pack(push, 1)
struct DnsHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};
#pragma pack(pop)

// DNS Answer structure (for A record)
#pragma pack(push, 1)
struct DnsAnswer {
    uint16_t name_ptr;
    uint16_t type;
    uint16_t class_;
    uint32_t ttl;
    uint16_t rdlength;
    uint32_t rdata;
};
#pragma pack(pop)

// Function to parse a domain name from DNS wire format
std::string parseDomainName(const std::vector<uint8_t>& buffer, size_t& offset) {
    std::string name;
    if (offset >= buffer.size()) {
        throw std::runtime_error("Offset out of bounds");
    }

    while (buffer[offset] != 0) {
        // Security: Prevent out-of-bounds reads
        if ((buffer[offset] & 0xC0) == 0xC0) {
            // Pointers in requests are not handled by this simple server
            throw std::runtime_error("Pointers in request QNAME not supported");
        }
        uint8_t length = buffer[offset];
        offset++;
        if (offset + length > buffer.size()) {
            throw std::runtime_error("Malformed domain name label");
        }
        if (!name.empty()) {
            name += ".";
        }
        name.append(reinterpret_cast<const char*>(&buffer[offset]), length);
        offset += length;
    }
    offset++; // Skip the null terminator of the domain name
    name += ".";
    return name;
}

void startServer() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP_ADDRESS, &serverAddr.sin_addr);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
        return;
    }

    std::cout << "C++ DNS Server listening on " << IP_ADDRESS << ":" << PORT << std::endl;

    while (true) {
        std::vector<uint8_t> buffer(BUFFER_SIZE);
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);

        int bytesReceived = recvfrom(serverSocket, reinterpret_cast<char*>(buffer.data()), buffer.size(), 0,
                                     (sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived <= 0) continue;
        buffer.resize(bytesReceived);

        try {
            if (buffer.size() < sizeof(DnsHeader)) continue;

            DnsHeader* header = reinterpret_cast<DnsHeader*>(buffer.data());
            header->id = ntohs(header->id);
            header->flags = ntohs(header->flags);
            header->qdcount = ntohs(header->qdcount);

            // Security: Handle only standard queries with one question
            if ((header->flags & 0x7800) != 0 || header->qdcount != 1) continue;

            size_t offset = sizeof(DnsHeader);
            std::string domainName = parseDomainName(buffer, offset);
            
            // Get QTYPE and QCLASS
            uint16_t qtype = ntohs(*reinterpret_cast<uint16_t*>(&buffer[offset]));
            uint16_t qclass = ntohs(*reinterpret_cast<uint16_t*>(&buffer[offset + 2]));
            size_t questionLength = offset - sizeof(DnsHeader) + 4;

            std::vector<uint8_t> response;
            DnsHeader responseHeader = *header;
            responseHeader.flags = htons(0x8180); // Response, no error
            responseHeader.ancount = 0;

            auto it = dnsRecords.find(domainName);
            if (it != dnsRecords.end() && qtype == 1 && qclass == 1) { // Found, A, IN
                responseHeader.ancount = htons(1);
                
                response.insert(response.end(), reinterpret_cast<uint8_t*>(&responseHeader), reinterpret_cast<uint8_t*>(&responseHeader) + sizeof(DnsHeader));
                response.insert(response.end(), buffer.begin() + sizeof(DnsHeader), buffer.begin() + sizeof(DnsHeader) + questionLength);
                
                DnsAnswer answer;
                answer.name_ptr = htons(0xC00C); // Pointer to domain name at offset 12
                answer.type = htons(1);          // A record
                answer.class_ = htons(1);        // IN class
                answer.ttl = htonl(60);          // TTL 60 seconds
                answer.rdlength = htons(4);      // 4 bytes for IPv4
                inet_pton(AF_INET, it->second.c_str(), &answer.rdata);

                response.insert(response.end(), reinterpret_cast<uint8_t*>(&answer), reinterpret_cast<uint8_t*>(&answer) + sizeof(DnsAnswer));
            } else { // Not found or unsupported type
                responseHeader.flags = htons(0x8183); // Name Error
                response.insert(response.end(), reinterpret_cast<uint8_t*>(&responseHeader), reinterpret_cast<uint8_t*>(&responseHeader) + sizeof(DnsHeader));
                response.insert(response.end(), buffer.begin() + sizeof(DnsHeader), buffer.begin() + sizeof(DnsHeader) + questionLength);
            }

            sendto(serverSocket, reinterpret_cast<char*>(response.data()), response.size(), 0, (sockaddr*)&clientAddr, clientAddrLen);

        } catch (const std::exception& e) {
            std::cerr << "Error handling request: " << e.what() << std::endl;
        }
    }

    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    std::cout << "--- C++ Simple DNS Server ---" << std::endl;
    std::cout << "This server will run indefinitely. Stop with Ctrl+C." << std::endl;
    std::cout << "--- 5 Test Cases to run manually ---" << std::endl;
    std::cout << "1. Known domain:      dig @127.0.0.1 -p 53530 test1.example.com A" << std::endl;
    std::cout << "2. Known domain:      dig @127.0.0.1 -p 53530 test2.example.com A" << std::endl;
    std::cout << "3. Known domain:      dig @127.0.0.1 -p 53530 secure.example.com A" << std::endl;
    std::cout << "4. Unknown domain:    dig @127.0.0.1 -p 53530 unknown.example.com A" << std::endl;
    std::cout << "5. Unsupported type:  dig @127.0.0.1 -p 53530 test1.example.com MX" << std::endl;
    std::cout << "----------------------------------" << std::endl;
    
    startServer();
    
    return 0;
}