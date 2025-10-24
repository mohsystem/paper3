#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

static std::vector<uint8_t> build_dns_query(const std::string& name, uint16_t qtype, uint16_t qid) {
    if (name.empty() || qtype == 0) throw std::invalid_argument("bad params");
    std::vector<uint8_t> out;
    out.reserve(64);
    // Header
    out.push_back((qid >> 8) & 0xFF); out.push_back(qid & 0xFF);
    out.push_back(0x01); out.push_back(0x00); // RD
    out.push_back(0x00); out.push_back(0x01); // QD=1
    out.push_back(0x00); out.push_back(0x00); // AN=0
    out.push_back(0x00); out.push_back(0x00); // NS=0
    out.push_back(0x00); out.push_back(0x00); // AR=0
    // QNAME
    size_t start = 0;
    while (start <= name.size()) {
        size_t dot = name.find('.', start);
        if (dot == std::string::npos) dot = name.size();
        size_t len = dot - start;
        if (len == 0 || len > 63) throw std::invalid_argument("label len");
        out.push_back((uint8_t)len);
        for (size_t i = start; i < dot; ++i) out.push_back((uint8_t)name[i]);
        if (dot == name.size()) break;
        start = dot + 1;
    }
    out.push_back(0x00);
    out.push_back((qtype >> 8) & 0xFF); out.push_back(qtype & 0xFF);
    out.push_back(0x00); out.push_back(0x01);
    if (out.size() > 512) throw std::runtime_error("query too big");
    return out;
}

static bool parse_qname(const std::vector<uint8_t>& data, size_t offset, std::string& name, size_t& next) {
    size_t i = offset;
    std::string res;
    bool first = true;
    while (true) {
        if (i >= data.size()) return false;
        uint8_t l = data[i++];
        if ((l & 0xC0) == 0xC0) return false; // no compression support in demo
        if (l == 0) break;
        if (l > 63 || i + l > data.size()) return false;
        if (!first) res.push_back('.');
        first = false;
        res.append((const char*)&data[i], (size_t)l);
        i += l;
    }
    name = res;
    next = i;
    return true;
}

static std::vector<uint8_t> build_dns_response(const std::vector<uint8_t>& req) {
    if (req.size() < 12 || req.size() > 512) return {};
    uint16_t id = (req[0] << 8) | req[1];
    uint16_t flags = (req[2] << 8) | req[3];
    uint16_t qd = (req[4] << 8) | req[5];
    if (qd != 1) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        return { (uint8_t)(id>>8),(uint8_t)id,(uint8_t)(rflags>>8),(uint8_t)rflags,0,0,0,0,0,0,0,0 };
    }
    std::string qname;
    size_t next = 0;
    if (!parse_qname(req, 12, qname, next)) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        return { (uint8_t)(id>>8),(uint8_t)id,(uint8_t)(rflags>>8),(uint8_t)rflags,0,0,0,0,0,0,0,0 };
    }
    if (next + 4 > req.size()) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 1;
        return { (uint8_t)(id>>8),(uint8_t)id,(uint8_t)(rflags>>8),(uint8_t)rflags,0,0,0,0,0,0,0,0 };
    }
    uint16_t qtype = (req[next] << 8) | req[next+1];
    uint16_t qclass = (req[next+2] << 8) | req[next+3];
    if (qclass != 1) {
        uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | 4;
        return { (uint8_t)(id>>8),(uint8_t)id,(uint8_t)(rflags>>8),(uint8_t)rflags,0,0,0,0,0,0,0,0 };
    }
    auto lower = qname;
    for (auto& ch : lower) ch = (char)std::tolower((unsigned char)ch);
    int rcode = 0;
    uint32_t ipv4 = 0;
    if (qtype == 1) {
        if (lower == "example.com") {
            inet_pton(AF_INET, "93.184.216.34", &ipv4);
        } else if (lower == "localhost") {
            inet_pton(AF_INET, "127.0.0.1", &ipv4);
        } else if (lower == "test.local") {
            inet_pton(AF_INET, "192.168.1.2", &ipv4);
        } else {
            rcode = 3;
        }
    } else {
        rcode = 4;
    }
    uint16_t rflags = 0x8000 | (flags & 0x0100) | 0x0400 | (rcode & 0xF);
    std::vector<uint8_t> out;
    out.reserve(128);
    out.push_back((uint8_t)(id>>8)); out.push_back((uint8_t)id);
    out.push_back((uint8_t)(rflags>>8)); out.push_back((uint8_t)rflags);
    out.push_back(0x00); out.push_back(0x01); // QD=1
    if (rcode == 0 && qtype == 1 && ipv4 != 0) { out.push_back(0x00); out.push_back(0x01); }
    else { out.push_back(0x00); out.push_back(0x00); }
    out.push_back(0x00); out.push_back(0x00);
    out.push_back(0x00); out.push_back(0x00);
    // question
    out.insert(out.end(), req.begin()+12, req.begin()+next+4);
    if (rcode == 0 && qtype == 1 && ipv4 != 0) {
        // NAME pointer
        out.push_back(0xC0); out.push_back(0x0C);
        // TYPE A
        out.push_back(0x00); out.push_back(0x01);
        // CLASS IN
        out.push_back(0x00); out.push_back(0x01);
        // TTL 60
        out.push_back(0x00); out.push_back(0x00); out.push_back(0x00); out.push_back(0x3C);
        // RDLENGTH 4
        out.push_back(0x00); out.push_back(0x04);
        // RDATA
        uint32_t net = ipv4; // already network order from inet_pton
        out.push_back(((uint8_t*)&net)[0]);
        out.push_back(((uint8_t*)&net)[1]);
        out.push_back(((uint8_t*)&net)[2]);
        out.push_back(((uint8_t*)&net)[3]);
    }
    if (out.size() > 512) return {};
    return out;
}

static void run_server(const std::string& bind_ip, uint16_t port, int max_requests) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons(port);
    inet_pton(AF_INET, bind_ip.c_str(), &addr.sin_addr);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); return; }
    int handled = 0;
    std::vector<uint8_t> buf(512);
    while (handled < max_requests) {
        sockaddr_in src{}; socklen_t sl = sizeof(src);
        ssize_t n = recvfrom(fd, buf.data(), buf.size(), 0, (sockaddr*)&src, &sl);
        if (n <= 0) continue;
        std::vector<uint8_t> req(buf.begin(), buf.begin()+n);
        std::vector<uint8_t> resp = build_dns_response(req);
        if (!resp.empty()) {
            sendto(fd, resp.data(), resp.size(), 0, (sockaddr*)&src, sl);
        }
        handled++;
    }
    close(fd);
}

static std::string parse_a_from_response(const std::vector<uint8_t>& resp) {
    if (resp.size() < 12) return "";
    uint16_t an = (resp[6] << 8) | resp[7];
    if (an < 1) return "";
    size_t i = 12;
    while (i < resp.size() && resp[i] != 0) {
        uint8_t l = resp[i];
        if (l & 0xC0) { if (i+1 >= resp.size()) return ""; i += 2; break; }
        i += 1 + l;
    }
    if (i >= resp.size()) return "";
    if (resp[i] == 0) i++;
    i += 4; // qtype+qclass
    if (i >= resp.size()) return "";
    if (resp[i] & 0xC0) i += 2;
    else {
        while (i < resp.size() && resp[i] != 0) { uint8_t l = resp[i]; i += 1 + l; }
        i++;
    }
    if (i + 10 > resp.size()) return "";
    uint16_t type = (resp[i] << 8) | resp[i+1]; i += 2;
    i += 2; // class
    i += 4; // ttl
    uint16_t rdlen = (resp[i] << 8) | resp[i+1]; i += 2;
    if (type != 1 || rdlen != 4 || i + 4 > resp.size()) return "";
    char ipbuf[INET_ADDRSTRLEN]{0};
    inet_ntop(AF_INET, (void*)&resp[i], ipbuf, sizeof(ipbuf));
    return std::string(ipbuf);
}

int main() {
    std::string ip = "127.0.0.1";
    uint16_t port = 53537;
    std::thread th([&]() { run_server(ip, port, 5); });
    th.detach();
    usleep(200000);
    std::pair<std::string,uint16_t> tests[5] = {
        {"example.com",1},
        {"localhost",1},
        {"test.local",1},
        {"unknown.example",1},
        {"example.com",28}
    };
    for (int i=0;i<5;i++) {
        std::vector<uint8_t> q = build_dns_query(tests[i].first, tests[i].second, 0x3333 + i);
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        timeval tv{2,0};
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        sockaddr_in dst{}; dst.sin_family = AF_INET; dst.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &dst.sin_addr);
        sendto(fd, q.data(), q.size(), 0, (sockaddr*)&dst, sizeof(dst));
        std::vector<uint8_t> buf(512);
        sockaddr_in src{}; socklen_t sl = sizeof(src);
        ssize_t n = recvfrom(fd, buf.data(), buf.size(), 0, (sockaddr*)&src, &sl);
        if (n > 0) {
            std::vector<uint8_t> resp(buf.begin(), buf.begin()+n);
            std::string a = parse_a_from_response(resp);
            std::cout << "CPP Test " << (i+1) << " " << tests[i].first << " type " << tests[i].second << " -> " << (a.empty()?"NOANSWER":a) << std::endl;
        } else {
            std::cout << "CPP Test " << (i+1) << " timeout" << std::endl;
        }
        close(fd);
    }
    usleep(300000);
    return 0;
}