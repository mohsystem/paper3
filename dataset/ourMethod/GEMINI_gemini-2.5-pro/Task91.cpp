#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstdint>
#include <cstring>
#include <stdexcept>

// Platform-specific socket headers
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

// Define constants
const int DNS_PORT = 53535;
const char* LISTEN_ADDR = "127.0.0.1";
const int BUFFER_SIZE = 512;

// Simple DNS record map
static std::map<std::string, std::string> dnsRecords = {
    {"example.com.", "93.184.216.34"},
    {"test.local.", "192.168.1.100"},
    {"hello.world.", "1.1.1.1"},
    {"another.test.", "8.8.8.8"},
    {"localhost.", "127.0.0.1"}
};

// DNS Header structure
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

// RAII wrapper for socket file descriptor
class Socket {
public:
#ifdef _WIN32
    using FdType = SOCKET;
    const FdType INVALID_FD = INVALID_SOCKET;
    Socket(FdType fd) : fd_(fd) {}
    ~Socket() { if (fd_ != INVALID_FD) closesocket(fd_); }
#else
    using FdType = int;
    const FdType INVALID_FD = -1;
    Socket(FdType fd) : fd_(fd) {}
    ~Socket() { if (fd_ >= 0) close(fd_); }
#endif
    FdType get() const { return fd_; }
private:
    FdType fd_;
};


// Function to parse a domain name from DNS format
std::string parseDomainName(const std::vector<uint8_t>& buffer, size_t& offset) {
    std::string name;
    if (offset >= buffer.size()) return "";

    while (offset < buffer.size() && buffer[offset] != 0) {
        if ((buffer[offset] & 0xC0) == 0xC0) {
            std::cerr << "Pointers in request QNAME are not supported." << std::endl;
            return "";
        }
        uint8_t length = buffer[offset];
        offset++;

        if (offset + length > buffer.size()) {
            std::cerr << "Malformed QNAME: length byte exceeds buffer." << std::endl;
            return "";
        }
        name.append(reinterpret_cast<const char*>(&buffer[offset]), length);
        offset += length;
        if (offset < buffer.size() && buffer[offset] != 0) {
            name += ".";
        }
    }
    if (offset < buffer.size() && buffer[offset] == 0) offset++;
    name += ".";
    return name;
}


void run_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
#endif

    Socket sock(socket(AF_INET, SOCK_DGRAM, 0));
    if (sock.get() == sock.INVALID_FD) {
        throw std::runtime_error("Socket creation failed");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, LISTEN_ADDR, &server_addr.sin_addr);

    if (bind(sock.get(), (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        throw std::runtime_error("Bind failed");
    }
    
    std::cout << "Starting C++ DNS server on " << LISTEN_ADDR << ":" << DNS_PORT << std::endl;

    while (true) {
        std::vector<uint8_t> buffer(BUFFER_SIZE);
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        // 1. Receive incoming DNS request
        ssize_t bytes_received = recvfrom(sock.get(), reinterpret_cast<char*>(buffer.data()), buffer.size(), 0,
                                          (struct sockaddr*)&client_addr, &client_len);

        if (bytes_received < 0) {
            perror("recvfrom failed");
            continue;
        }

        buffer.resize(bytes_received);

        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        std::cout << "\nReceived request from " << client_ip_str << ":" << ntohs(client_addr.sin_port) << std::endl;

        // 2. Parse the received data
        if (buffer.size() < sizeof(DnsHeader)) {
            std::cerr << "Request too short for a DNS header." << std::endl;
            continue;
        }
        
        DnsHeader* header = reinterpret_cast<DnsHeader*>(buffer.data());
        if (ntohs(header->qdcount) != 1) continue;

        size_t offset = sizeof(DnsHeader);
        std::string domain_name = parseDomainName(buffer, offset);
        if (domain_name.empty()) continue;
        
        if (offset + 4 > buffer.size()) {
             std::cerr << "Packet too short for QTYPE/QCLASS. Skipping." << std::endl;
             continue;
        }
        uint16_t qtype = (buffer[offset] << 8) | buffer[offset+1];
        uint16_t qclass = (buffer[offset+2] << 8) | buffer[offset+3];

        if (qtype != 1 || qclass != 1) continue;
        std::cout << "Query for: " << domain_name << " (A, IN)" << std::endl;
        size_t question_len = offset + 4 - sizeof(DnsHeader);

        // 3. Resolve & 4. Construct DNS response
        std::vector<uint8_t> response;
        response.insert(response.end(), buffer.begin(), buffer.begin() + sizeof(DnsHeader) + question_len);
        DnsHeader* res_header = reinterpret_cast<DnsHeader*>(response.data());
        
        auto it = dnsRecords.find(domain_name);
        if (it != dnsRecords.end()) {
            std::cout << "Resolved " << domain_name << " to " << it->second << std::endl;
            res_header->flags = htons(0x8180);
            res_header->ancount = htons(1);
            
            // Append Answer: Pointer, Type, Class, TTL, RDLength, RData
            response.push_back(0xc0); response.push_back(0x0c);
            response.push_back(0x00); response.push_back(0x01); // Type A
            response.push_back(0x00); response.push_back(0x01); // Class IN
            uint32_t ttl_n = htonl(60);
            response.insert(response.end(), (uint8_t*)&ttl_n, (uint8_t*)&ttl_n + 4);
            uint16_t rdlen_n = htons(4);
            response.insert(response.end(), (uint8_t*)&rdlen_n, (uint8_t*)&rdlen_n + 2);
            in_addr ip_addr{};
            inet_pton(AF_INET, it->second.c_str(), &ip_addr);
            response.insert(response.end(), (uint8_t*)&ip_addr, (uint8_t*)&ip_addr + 4);
        } else {
            std::cout << "Could not resolve: " << domain_name << std::endl;
            res_header->flags = htons(0x8183); // Name Error
            res_header->ancount = htons(0);
        }

        // 5. Send response
        sendto(sock.get(), reinterpret_cast<char*>(response.data()), response.size(), 0,
               (struct sockaddr*)&client_addr, client_len);
        std::cout << "Sent response to " << client_ip_str << ":" << ntohs(client_addr.sin_port) << std::endl;
    }
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    // 5 test cases explained. To test, compile and run this server, then use a DNS client like `dig` or `nslookup`.
    // Example commands:
    // 1. dig @127.0.0.1 -p 53535 example.com
    // 2. dig @127.0.0.1 -p 53535 test.local
    // 3. dig @127.0.0.1 -p 53535 hello.world
    // 4. dig @127.0.0.1 -p 53535 not.found
    // 5. dig @127.0.0.1 -p 53535 localhost
    try {
        run_server();
    } catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}