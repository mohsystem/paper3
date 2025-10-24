#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <cstring>
#include <atomic>

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

#pragma pack(push, 1)
struct DnsHeader {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

struct DnsQuestionTrailer {
    uint16_t qtype;
    uint16_t qclass;
};

struct DnsAnswer {
    uint16_t name;
    uint16_t type;
    uint16_t a_class;
    uint32_t ttl;
    uint16_t rdlength;
    uint32_t rdata;
};
#pragma pack(pop)

const int PORT = 5353;
const char* IP = "127.0.0.1";
std::atomic<bool> server_running(true);
std::map<std::string, std::string> dns_records = {
    {"test1.dns.local", "1.1.1.1"},
    {"test2.dns.local", "2.2.2.2"},
    {"test3.dns.local", "3.3.3.3"}
};

std::string parse_domain_name(const char* buffer, int& offset) {
    std::string name;
    while (buffer[offset] != 0) {
        uint8_t len = buffer[offset++];
        if (!name.empty()) {
            name += '.';
        }
        name.append(buffer + offset, len);
        offset += len;
    }
    offset++; // Skip the null terminator
    return name;
}

void encode_domain_name(char* buffer, int& offset, const std::string& domain) {
    size_t start = 0;
    for (size_t i = 0; i < domain.length(); ++i) {
        if (domain[i] == '.') {
            buffer[offset++] = i - start;
            memcpy(buffer + offset, domain.c_str() + start, i - start);
            offset += (i - start);
            start = i + 1;
        }
    }
    buffer[offset++] = domain.length() - start;
    memcpy(buffer + offset, domain.c_str() + start, domain.length() - start);
    offset += (domain.length() - start);
    buffer[offset++] = 0;
}

void run_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return;
    }
#endif

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed\n";
        return;
    }
    
#ifdef _WIN32
    DWORD timeout = 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
#endif

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr);

    if (bind(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed\n";
        closesocket(sock);
        return;
    }

    std::cout << "CPP DNS Server listening on " << IP << ":" << PORT << std::endl;

    char buffer[512];
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (server_running) {
        int bytes_received = recvfrom(sock, buffer, sizeof(buffer), 0, (sockaddr*)&client_addr, &client_len);
        if (bytes_received > 0) {
            DnsHeader* req_header = (DnsHeader*)buffer;
            int offset = sizeof(DnsHeader);
            std::string domain = parse_domain_name(buffer, offset);
            DnsQuestionTrailer* q_trailer = (DnsQuestionTrailer*)(buffer + offset);

            char response_buffer[512];
            DnsHeader* res_header = (DnsHeader*)response_buffer;
            res_header->id = req_header->id;
            res_header->qdcount = htons(1);
            res_header->nscount = 0;
            res_header->arcount = 0;
            
            int response_size = sizeof(DnsHeader);
            encode_domain_name(response_buffer, response_size, domain);
            DnsQuestionTrailer* res_q_trailer = (DnsQuestionTrailer*)(response_buffer + response_size);
            res_q_trailer->qtype = q_trailer->qtype;
            res_q_trailer->qclass = q_trailer->qclass;
            response_size += sizeof(DnsQuestionTrailer);

            auto it = dns_records.find(domain);
            if (it != dns_records.end() && ntohs(q_trailer->qtype) == 1) { // A record
                res_header->flags = htons(0x8180);
                res_header->ancount = htons(1);

                DnsAnswer* answer = (DnsAnswer*)(response_buffer + response_size);
                answer->name = htons(0xC00C);
                answer->type = htons(1);
                answer->a_class = htons(1);
                answer->ttl = htonl(60);
                answer->rdlength = htons(4);
                inet_pton(AF_INET, it->second.c_str(), &answer->rdata);
                response_size += sizeof(DnsAnswer);
            } else { // NXDOMAIN
                res_header->flags = htons(0x8183);
                res_header->ancount = htons(0);
            }
            sendto(sock, response_buffer, response_size, 0, (sockaddr*)&client_addr, client_len);
        }
    }
    closesocket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "CPP DNS Server stopped." << std::endl;
}

void run_client_test(const std::string& hostname) {
    std::cout << "\n--- Testing: " << hostname << " ---" << std::endl;
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) return;

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, IP, &server_addr.sin_addr);

    char query_buffer[512];
    memset(query_buffer, 0, sizeof(query_buffer));
    DnsHeader* header = (DnsHeader*)query_buffer;
    header->id = htons(0x1234);
    header->flags = htons(0x0100);
    header->qdcount = htons(1);
    
    int offset = sizeof(DnsHeader);
    encode_domain_name(query_buffer, offset, hostname);
    
    DnsQuestionTrailer* q_trailer = (DnsQuestionTrailer*)(query_buffer + offset);
    q_trailer->qtype = htons(1); // A
    q_trailer->qclass = htons(1); // IN
    offset += sizeof(DnsQuestionTrailer);

    sendto(sock, query_buffer, offset, 0, (sockaddr*)&server_addr, sizeof(server_addr));
    
    char response_buffer[512];
    int bytes = recvfrom(sock, response_buffer, sizeof(response_buffer), 0, NULL, NULL);

    std::cout << "Query sent for: " << hostname << std::endl;
    if (bytes > 0) {
        DnsHeader* res_header = (DnsHeader*)response_buffer;
        uint16_t flags = ntohs(res_header->flags);
        printf("Flags: 0x%04x\n", flags);
        if ((flags & 0x000F) == 3) {
            std::cout << "Response: NXDOMAIN (Not Found)" << std::endl;
        } else if (ntohs(res_header->ancount) > 0) {
            DnsAnswer* ans = (DnsAnswer*)(response_buffer + bytes - sizeof(DnsAnswer));
            in_addr ip_addr;
            ip_addr.s_addr = ans->rdata;
            std::cout << "Resolved IP: " << inet_ntoa(ip_addr) << std::endl;
        }
    } else {
        std::cout << "No response from server." << std::endl;
    }
    closesocket(sock);
}


int main() {
    std::thread server_thread(run_server);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // 5 Test cases
    run_client_test("test1.dns.local");
    run_client_test("test2.dns.local");
    run_client_test("unknown.dns.local");
    run_client_test("test3.dns.local");
    run_client_test("another.unknown.com");

    server_running = false;
    server_thread.join();
    std::cout << "\nAll tests completed." << std::endl;
    return 0;
}