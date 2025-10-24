#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <set>
#include <string>
#include <thread>
#include <vector>

static const int TYPE_A = 1;
static const int TYPE_AAAA = 28;

std::vector<uint8_t> encode_name(const std::string& name) {
    std::vector<uint8_t> out;
    if (name.empty()) {
        out.push_back(0);
        return out;
    }
    size_t start = 0, end = 0;
    while (end <= name.size()) {
        if (end == name.size() || name[end] == '.') {
            size_t len = end - start;
            if (len == 0 || len > 63) throw std::runtime_error("Invalid label length");
            out.push_back(static_cast<uint8_t>(len));
            out.insert(out.end(), name.begin() + start, name.begin() + end);
            start = end + 1;
        }
        end++;
    }
    out.push_back(0);
    return out;
}

std::string decode_name(const std::vector<uint8_t>& data, size_t& offset) {
    std::string name;
    size_t o = offset;
    bool jumped = false;
    size_t jump_end = 0;
    std::set<size_t> seen;

    while (true) {
        if (o >= data.size()) throw std::runtime_error("Truncated name");
        uint8_t len = data[o];
        if ((len & 0xC0) == 0xC0) {
            if (o + 1 >= data.size()) throw std::runtime_error("Bad pointer");
            size_t ptr = ((len & 0x3F) << 8) | data[o + 1];
            if (seen.count(ptr)) throw std::runtime_error("Pointer loop");
            seen.insert(ptr);
            if (!jumped) {
                jump_end = o + 2;
                jumped = true;
            }
            o = ptr;
            continue;
        } else if (len == 0) {
            o++;
            break;
        } else {
            o++;
            if (o + len > data.size()) throw std::runtime_error("Label out of range");
            if (!name.empty()) name.push_back('.');
            name.append(reinterpret_cast<const char*>(&data[o]), len);
            o += len;
        }
    }
    offset = jumped ? jump_end : o;
    return name;
}

struct DnsQuery {
    uint16_t id;
    uint16_t flags;
    std::string qname;
    uint16_t qtype;
    uint16_t qclass;
    size_t question_end;
};

DnsQuery parse_query(const std::vector<uint8_t>& data) {
    if (data.size() < 12) throw std::runtime_error("Header too short");
    DnsQuery q{};
    q.id = (data[0] << 8) | data[1];
    q.flags = (data[2] << 8) | data[3];
    uint16_t qd = (data[4] << 8) | data[5];
    if (qd < 1) throw std::runtime_error("No questions");
    size_t off = 12;
    q.qname = decode_name(data, off);
    if (off + 4 > data.size()) throw std::runtime_error("Truncated question");
    q.qtype = (data[off] << 8) | data[off + 1];
    q.qclass = (data[off + 2] << 8) | data[off + 3];
    q.question_end = off + 4;
    return q;
}

std::vector<std::vector<uint8_t>> resolve(const std::string& name, uint16_t qtype) {
    std::vector<std::vector<uint8_t>> out;
    try {
        if (strcasecmp(name.c_str(), "localhost") == 0) {
            if (qtype == TYPE_A) {
                std::vector<uint8_t> a(4);
                inet_pton(AF_INET, "127.0.0.1", a.data());
                out.push_back(a);
            }
            if (qtype == TYPE_AAAA) {
                std::vector<uint8_t> a(16);
                inet_pton(AF_INET6, "::1", a.data());
                out.push_back(a);
            }
            return out;
        }
        addrinfo hints{};
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_DGRAM;
        hints.ai_family = (qtype == TYPE_A) ? AF_INET : (qtype == TYPE_AAAA ? AF_INET6 : AF_UNSPEC);
        addrinfo* res = nullptr;
        if (getaddrinfo(name.c_str(), nullptr, &hints, &res) == 0 && res) {
            std::set<std::string> seen;
            for (addrinfo* p = res; p; p = p->ai_next) {
                char buf[INET6_ADDRSTRLEN];
                void* addrptr = nullptr;
                size_t len = 0;
                if (p->ai_family == AF_INET && qtype == TYPE_A) {
                    addrptr = &((sockaddr_in*)p->ai_addr)->sin_addr;
                    len = 4;
                } else if (p->ai_family == AF_INET6 && qtype == TYPE_AAAA) {
                    addrptr = &((sockaddr_in6*)p->ai_addr)->sin6_addr;
                    len = 16;
                }
                if (addrptr) {
                    inet_ntop(p->ai_family, addrptr, buf, sizeof(buf));
                    if (!seen.count(buf)) {
                        seen.insert(buf);
                        std::vector<uint8_t> a(len);
                        memcpy(a.data(), addrptr, len);
                        out.push_back(a);
                    }
                }
            }
            freeaddrinfo(res);
        }
    } catch (...) {}
    return out;
}

std::vector<uint8_t> build_response(const std::vector<uint8_t>& request) {
    try {
        DnsQuery q = parse_query(request);
        bool supported = (q.qclass == 1) && (q.qtype == TYPE_A || q.qtype == TYPE_AAAA);
        auto answers = supported ? resolve(q.qname, q.qtype) : std::vector<std::vector<uint8_t>>{};
        int rcode = supported ? (answers.empty() ? 3 : 0) : 4;

        uint16_t flags = 0x8000; // QR=1
        flags |= (q.flags & 0x7800); // opcode
        flags |= (q.flags & 0x0100); // RD
        flags |= (rcode & 0xF);

        std::vector<uint8_t> resp;
        resp.reserve(512);
        auto push16 = [&](uint16_t v){ resp.push_back((v >> 8) & 0xFF); resp.push_back(v & 0xFF); };
        auto push32 = [&](uint32_t v){ resp.push_back((v >> 24) & 0xFF); resp.push_back((v >> 16) & 0xFF); resp.push_back((v >> 8) & 0xFF); resp.push_back(v & 0xFF); };

        push16(q.id);
        push16(flags);
        push16(1);
        push16((rcode == 0) ? (uint16_t)answers.size() : 0);
        push16(0);
        push16(0);
        // question
        resp.insert(resp.end(), request.begin() + 12, request.begin() + q.question_end);

        if (rcode == 0) {
            for (auto& rd : answers) {
                push16(0xC00C);             // name pointer
                push16(q.qtype);            // type
                push16(1);                  // class IN
                push32(60);                 // TTL
                push16((uint16_t)rd.size());// RDLENGTH
                resp.insert(resp.end(), rd.begin(), rd.end());
            }
        }
        if (resp.size() > 512) resp.resize(512);
        return resp;
    } catch (...) {
        std::vector<uint8_t> resp;
        resp.reserve(12);
        uint16_t id = 0;
        if (request.size() >= 2) id = (request[0] << 8) | request[1];
        resp.push_back((id >> 8) & 0xFF); resp.push_back(id & 0xFF);
        resp.push_back(0x80); resp.push_back(0x02); // SERVFAIL
        for (int i = 0; i < 8; i++) resp.push_back(0);
        return resp;
    }
}

std::vector<uint8_t> build_query(uint16_t id, const std::string& name, uint16_t qtype) {
    std::vector<uint8_t> q;
    auto push16 = [&](uint16_t v){ q.push_back((v >> 8) & 0xFF); q.push_back(v & 0xFF); };
    push16(id);
    push16(0x0100);
    push16(1);
    push16(0);
    push16(0);
    push16(0);
    auto nm = encode_name(name);
    q.insert(q.end(), nm.begin(), nm.end());
    push16(qtype);
    push16(1);
    return q;
}

void run_dns_server(const std::string& bindIp, int port, int durationSeconds) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return;
    timeval tv{0, 500000};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    int reuse = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, bindIp.c_str(), &addr.sin_addr);
    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return;
    }
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds(durationSeconds);
    std::vector<uint8_t> buf(2048);
    while (std::chrono::steady_clock::now() < end) {
        sockaddr_in cli{};
        socklen_t clen = sizeof(cli);
        ssize_t n = recvfrom(fd, buf.data(), buf.size(), 0, (sockaddr*)&cli, &clen);
        if (n < 0) continue;
        std::vector<uint8_t> req(buf.begin(), buf.begin() + n);
        std::vector<uint8_t> resp = build_response(req);
        if (!resp.empty()) {
            sendto(fd, resp.data(), resp.size(), 0, (sockaddr*)&cli, clen);
        }
    }
    close(fd);
}

static const char* rcode_name(int r) {
    switch (r) {
        case 0: return "NOERROR";
        case 2: return "SERVFAIL";
        case 3: return "NXDOMAIN";
        case 4: return "NOTIMP";
        default: return "RCODE";
    }
}

int main() {
    std::thread server([](){ run_dns_server("127.0.0.1", 8055, 5); });
    server.detach();

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    timeval tv{1, 500000};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    sockaddr_in srv{};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(8055);
    inet_pton(AF_INET, "127.0.0.1", &srv.sin_addr);

    std::string names[5] = {"localhost", "localhost", "example.com", "nonexistent.invalid", "example.com"};
    int types[5] = {TYPE_A, TYPE_AAAA, TYPE_A, TYPE_A, 16};

    for (int i = 0; i < 5; ++i) {
        auto q = build_query(0x3000 + i, names[i], types[i]);
        sendto(fd, q.data(), q.size(), 0, (sockaddr*)&srv, sizeof(srv));
        uint8_t buf[2048];
        sockaddr_in from{};
        socklen_t flen = sizeof(from);
        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (sockaddr*)&from, &flen);
        if (n >= 12) {
            int id = (buf[0] << 8) | buf[1];
            int flags = (buf[2] << 8) | buf[3];
            int an = (buf[6] << 8) | buf[7];
            int rcode = flags & 0xF;
            std::cout << "Test " << (i + 1) << ": id=" << id << " answers=" << an << " rcode=" << rcode_name(rcode) << "\n";
        } else {
            std::cout << "Test " << (i + 1) << ": timeout/empty\n";
        }
    }

    close(fd);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}