#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct ParsedQuery {
    uint16_t id;
    uint16_t flags;
    std::string qname;
    uint16_t qtype;
    uint16_t qclass;
    size_t questionEnd;
};

static uint16_t rd16(const uint8_t* p) { return (uint16_t)((p[0] << 8) | p[1]); }
static void wr16(std::vector<uint8_t>& out, uint16_t v) { out.push_back((v >> 8) & 0xFF); out.push_back(v & 0xFF); }
static void wr32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back((v >> 24) & 0xFF); out.push_back((v >> 16) & 0xFF); out.push_back((v >> 8) & 0xFF); out.push_back(v & 0xFF);
}

ParsedQuery parseDnsQuery(const std::vector<uint8_t>& data) {
    if (data.size() < 12) throw std::runtime_error("short");
    ParsedQuery pq{};
    pq.id = rd16(&data[0]);
    pq.flags = rd16(&data[2]);
    uint16_t qd = rd16(&data[4]);
    if (qd < 1) throw std::runtime_error("no q");
    size_t idx = 12;
    std::string name;
    while (idx < data.size()) {
        uint8_t ln = data[idx++];
        if (ln == 0) break;
        if (idx + ln > data.size()) throw std::runtime_error("bad qname");
        if (!name.empty()) name.push_back('.');
        name.append(reinterpret_cast<const char*>(&data[idx]), ln);
        idx += ln;
    }
    if (idx + 4 > data.size()) throw std::runtime_error("trunc q");
    pq.qname = name;
    pq.qtype = rd16(&data[idx]);
    pq.qclass = rd16(&data[idx + 2]);
    pq.questionEnd = idx + 4;
    return pq;
}

std::vector<uint8_t> buildDnsResponse(const std::vector<uint8_t>& request, const std::map<std::string, std::string>& hosts) {
    ParsedQuery pq = parseDnsQuery(request);
    bool rd = (pq.flags & 0x0100) != 0;
    uint16_t opcode = pq.flags & 0x7800;
    std::string key = pq.qname;
    for (auto& c : key) c = (char)tolower(c);
    std::string ip;
    if (pq.qclass == 1 && pq.qtype == 1) {
        auto it = hosts.find(key);
        if (it != hosts.end()) ip = it->second;
    }
    int rcode = ip.empty() ? 3 : 0;
    std::vector<uint8_t> out;
    // header
    wr16(out, pq.id);
    wr16(out, (uint16_t)(0x8000 | opcode | 0x0400 | (rd ? 0x0100 : 0) | (rcode & 0xF)));
    wr16(out, 1);
    wr16(out, ip.empty() ? 0 : 1);
    wr16(out, 0);
    wr16(out, 0);
    // question
    out.insert(out.end(), request.begin() + 12, request.begin() + (long)pq.questionEnd);
    if (!ip.empty()) {
        // answer
        wr16(out, 0xC00C);
        wr16(out, 1);
        wr16(out, 1);
        wr32(out, 60);
        wr16(out, 4);
        // ip
        int b0, b1, b2, b3;
        if (sscanf(ip.c_str(), "%d.%d.%d.%d", &b0, &b1, &b2, &b3) != 4) throw std::runtime_error("bad ip");
        out.push_back((uint8_t)(b0 & 0xFF));
        out.push_back((uint8_t)(b1 & 0xFF));
        out.push_back((uint8_t)(b2 & 0xFF));
        out.push_back((uint8_t)(b3 & 0xFF));
    }
    return out;
}

std::vector<uint8_t> buildServfail(const std::vector<uint8_t>& request) {
    try {
        ParsedQuery pq = parseDnsQuery(request);
        bool rd = (pq.flags & 0x0100) != 0;
        std::vector<uint8_t> out;
        wr16(out, pq.id);
        wr16(out, (uint16_t)(0x8000 | (pq.flags & 0x7800) | (rd ? 0x0100 : 0) | 2));
        wr16(out, 1); wr16(out, 0); wr16(out, 0); wr16(out, 0);
        out.insert(out.end(), request.begin() + 12, request.begin() + (long)pq.questionEnd);
        return out;
    } catch (...) {
        std::vector<uint8_t> out;
        wr16(out, 0); wr16(out, 0x8182); wr16(out, 0); wr16(out, 0); wr16(out, 0); wr16(out, 0);
        return out;
    }
}

std::vector<uint8_t> buildDnsQuery(const std::string& name, uint16_t qtype, bool rd) {
    std::vector<uint8_t> out;
    uint16_t id = (uint16_t)(::time(nullptr) & 0xFFFF);
    wr16(out, id);
    wr16(out, rd ? 0x0100 : 0);
    wr16(out, 1); wr16(out, 0); wr16(out, 0); wr16(out, 0);
    size_t start_len = out.size();
    size_t pos = 0;
    while (pos < name.size()) {
        size_t dot = name.find('.', pos);
        if (dot == std::string::npos) dot = name.size();
        size_t len = dot - pos;
        out.push_back((uint8_t)len);
        out.insert(out.end(), name.begin() + (long)pos, name.begin() + (long)dot);
        pos = dot + 1;
    }
    out.push_back(0);
    wr16(out, qtype);
    wr16(out, 1);
    return out;
}

void runUdpDnsServer(const std::string& ip, uint16_t port, const std::map<std::string, std::string>& hosts, int maxRequests) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    int handled = 0;
    std::vector<uint8_t> buf(1500);
    while (maxRequests <= 0 || handled < maxRequests) {
        sockaddr_in cli{};
        socklen_t clen = sizeof(cli);
        ssize_t n = recvfrom(fd, buf.data(), buf.size(), 0, (sockaddr*)&cli, &clen);
        if (n <= 0) break;
        std::vector<uint8_t> req(buf.begin(), buf.begin() + n);
        std::vector<uint8_t> resp;
        try {
            resp = buildDnsResponse(req, hosts);
        } catch (...) {
            resp = buildServfail(req);
        }
        sendto(fd, resp.data(), resp.size(), 0, (sockaddr*)&cli, clen);
        handled++;
    }
    close(fd);
}

int main() {
    std::map<std::string, std::string> hosts{
        {"example.com", "93.184.216.34"},
        {"localhost", "127.0.0.1"},
        {"github.com", "140.82.113.3"},
        {"test.local", "10.0.0.1"},
        {"service.internal", "192.168.1.100"},
    };
    const char* tests[5] = {"example.com", "unknown.example", "github.com", "localhost", "test.local"};
    for (int i = 0; i < 5; ++i) {
        auto q = buildDnsQuery(tests[i], 1, true);
        auto r = buildDnsResponse(q, hosts);
        std::cout << "Q: " << tests[i] << " resp_len: " << r.size() << "\n";
    }
    // Example: runUdpDnsServer("127.0.0.1", 5353, hosts, 5);
    return 0;
}