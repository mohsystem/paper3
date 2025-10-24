#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <future>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// DNS constants
static const size_t DNS_MAX_UDP_SIZE = 512;
static const uint16_t DNS_CLASS_IN = 1;
static const uint16_t DNS_TYPE_A = 1;
static const uint16_t DNS_TYPE_AAAA = 28;
static const uint16_t DNS_TYPE_TXT = 16;

// Utility functions for safe network byte order conversions
static uint16_t read_u16(const uint8_t* buf, size_t len, size_t off, bool& ok) {
    if (off + 2 > len) {
        ok = false;
        return 0;
    }
    ok = true;
    return static_cast<uint16_t>((buf[off] << 8) | buf[off + 1]);
}
static uint32_t read_u32(const uint8_t* buf, size_t len, size_t off, bool& ok) {
    if (off + 4 > len) {
        ok = false;
        return 0;
    }
    ok = true;
    return (static_cast<uint32_t>(buf[off]) << 24) |
           (static_cast<uint32_t>(buf[off + 1]) << 16) |
           (static_cast<uint32_t>(buf[off + 2]) << 8) |
           (static_cast<uint32_t>(buf[off + 3]));
}
static void write_u16(std::vector<uint8_t>& out, uint16_t v) {
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}
static void write_u32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((v >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(v & 0xFF));
}

// Parse a domain name with compression handling, with strict bounds and loop limits
static bool parse_domain_name(const uint8_t* buf, size_t len, size_t& offset, std::string& out_name) {
    out_name.clear();
    size_t off = offset;
    size_t jumps = 0;
    const size_t MAX_JUMPS = 20; // prevent loops
    bool jumped = false;
    std::string result;

    while (true) {
        if (off >= len) return false;
        uint8_t labellen = buf[off];

        if ((labellen & 0xC0) == 0xC0) {
            // Pointer
            if (off + 1 >= len) return false;
            uint16_t ptr = static_cast<uint16_t>(((labellen & 0x3F) << 8) | buf[off + 1]);
            if (ptr >= len) return false;
            if (++jumps > MAX_JUMPS) return false;
            off = ptr;
            jumped = true;
            continue;
        } else if (labellen == 0) {
            // End of name
            if (!jumped) {
                offset = off + 1; // consume null byte
            }
            break;
        } else {
            // Label
            if (labellen > 63) return false;
            if (off + 1 + labellen > len) return false;
            if (!result.empty()) result.push_back('.');
            for (size_t i = 0; i < labellen; ++i) {
                char c = static_cast<char>(buf[off + 1 + i]);
                // Allow letters, digits, hyphen; others will be included but lower-cased
                result.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
            }
            off += 1 + labellen;
            if (!jumped) {
                // advance normal offset as well
                offset = off;
            }
        }
    }

    // Enforce maximum name length
    if (result.size() > 255) return false;
    out_name = result;
    return true;
}

static bool write_domain_name(std::vector<uint8_t>& out, const std::string& name) {
    if (name.empty()) {
        out.push_back(0);
        return true;
    }
    if (name.size() > 255) return false;
    size_t start = 0;
    size_t end;
    while (start < name.size()) {
        end = name.find('.', start);
        if (end == std::string::npos) end = name.size();
        size_t labellen = end - start;
        if (labellen == 0 || labellen > 63) return false;
        out.push_back(static_cast<uint8_t>(labellen));
        for (size_t i = start; i < end; ++i) {
            out.push_back(static_cast<uint8_t>(name[i]));
        }
        start = end + 1;
    }
    out.push_back(0);
    return true;
}

struct DNSQuestion {
    std::string qname;
    uint16_t qtype;
    uint16_t qclass;
};

static bool parse_question(const uint8_t* buf, size_t len, size_t& offset, DNSQuestion& q) {
    if (!parse_domain_name(buf, len, offset, q.qname)) return false;
    bool ok = true;
    q.qtype = read_u16(buf, len, offset, ok);
    if (!ok) return false;
    offset += 2;
    q.qclass = read_u16(buf, len, offset, ok);
    if (!ok) return false;
    offset += 2;
    return true;
}

struct DNSAnswer {
    uint16_t type;
    uint16_t cls;
    uint32_t ttl;
    std::vector<uint8_t> rdata;
};

// Minimal local resolver for demonstration; does not query external network
static bool resolve_locally(const std::string& qname, uint16_t qtype, std::vector<DNSAnswer>& answers) {
    answers.clear();
    std::string name = qname;
    // basic mapping
    if (name == "localhost") {
        if (qtype == DNS_TYPE_A || qtype == 255 /*ANY*/) {
            DNSAnswer a;
            a.type = DNS_TYPE_A;
            a.cls = DNS_CLASS_IN;
            a.ttl = 60;
            a.rdata = {127, 0, 0, 1};
            answers.push_back(a);
        }
        if (qtype == DNS_TYPE_AAAA || qtype == 255) {
            DNSAnswer aaaa;
            aaaa.type = DNS_TYPE_AAAA;
            aaaa.cls = DNS_CLASS_IN;
            aaaa.ttl = 60;
            aaaa.rdata.resize(16, 0);
            aaaa.rdata[15] = 1; // ::1
            answers.push_back(aaaa);
        }
        return !answers.empty();
    } else if (name == "example.com") {
        if (qtype == DNS_TYPE_A || qtype == 255) {
            DNSAnswer a;
            a.type = DNS_TYPE_A;
            a.cls = DNS_CLASS_IN;
            a.ttl = 300;
            a.rdata = {93, 184, 216, 34};
            answers.push_back(a);
        }
        return !answers.empty();
    } else if (name == "test.local") {
        if (qtype == DNS_TYPE_A || qtype == 255) {
            DNSAnswer a;
            a.type = DNS_TYPE_A;
            a.cls = DNS_CLASS_IN;
            a.ttl = 120;
            a.rdata = {192, 0, 2, 1};
            answers.push_back(a);
        }
        return !answers.empty();
    } else if (name == "ipv6.local") {
        if (qtype == DNS_TYPE_AAAA || qtype == 255) {
            DNSAnswer aaaa;
            aaaa.type = DNS_TYPE_AAAA;
            aaaa.cls = DNS_CLASS_IN;
            aaaa.ttl = 120;
            aaaa.rdata.resize(16, 0);
            aaaa.rdata[15] = 1; // ::1
            answers.push_back(aaaa);
        }
        return !answers.empty();
    } else if (name == "txt.local") {
        if (qtype == DNS_TYPE_TXT || qtype == 255) {
            const std::string s = "v=demo";
            if (s.size() > 255) return false;
            DNSAnswer txt;
            txt.type = DNS_TYPE_TXT;
            txt.cls = DNS_CLASS_IN;
            txt.ttl = 120;
            txt.rdata.push_back(static_cast<uint8_t>(s.size()));
            for (char c : s) txt.rdata.push_back(static_cast<uint8_t>(c));
            answers.push_back(txt);
            return true;
        }
        return false;
    }
    return false; // unknown
}

static std::vector<uint8_t> build_error_response(const uint8_t* req, size_t req_len, uint16_t rcode) {
    std::vector<uint8_t> resp;
    if (req_len < 12) return resp;
    resp.reserve(64);
    // Copy ID
    resp.push_back(req[0]);
    resp.push_back(req[1]);
    // Flags: QR=1, RCODE as provided, preserve RD
    uint16_t flags = static_cast<uint16_t>((req[2] << 8) | req[3]);
    flags |= 0x8000;       // QR=1
    flags &= ~0x0200;      // clear TC
    flags &= ~0x0010;      // clear RA
    flags &= ~0x000F;      // clear RCODE
    flags |= (rcode & 0x000F);
    write_u16(resp, flags);
    // QDCOUNT
    resp.push_back(req[4]);
    resp.push_back(req[5]);
    // ANCOUNT, NSCOUNT, ARCOUNT = 0
    write_u16(resp, 0);
    write_u16(resp, 0);
    write_u16(resp, 0);
    // Echo question if present safely
    size_t off = 12;
    // Try to copy one question safely
    size_t start_q = off;
    std::string dummy;
    size_t off_copy = off;
    if (parse_domain_name(req, req_len, off_copy, dummy)) {
        // domain bytes are from start_q up to off_copy
        if (off_copy + 4 <= req_len) {
            // copy name
            resp.insert(resp.end(), req + start_q, req + off_copy);
            // copy QTYPE, QCLASS
            resp.insert(resp.end(), req + off_copy, req + off_copy + 4);
        }
    }
    return resp;
}

// Build a DNS response for a single-question query
static std::vector<uint8_t> handle_dns_request(const uint8_t* req, size_t req_len) {
    std::vector<uint8_t> response;
    if (req_len < 12 || req_len > DNS_MAX_UDP_SIZE) {
        return build_error_response(req, req_len, 1); // FORMERR
    }
    bool ok = true;
    uint16_t id = read_u16(req, req_len, 0, ok);
    if (!ok) return build_error_response(req, req_len, 1);
    uint16_t flags = read_u16(req, req_len, 2, ok);
    if (!ok) return build_error_response(req, req_len, 1);
    uint16_t qdcount = read_u16(req, req_len, 4, ok);
    if (!ok) return build_error_response(req, req_len, 1);
    uint16_t ancount = read_u16(req, req_len, 6, ok);
    uint16_t nscount = read_u16(req, req_len, 8, ok);
    uint16_t arcount = read_u16(req, req_len, 10, ok);
    (void)ancount; (void)nscount; (void)arcount;

    if (qdcount != 1) {
        return build_error_response(req, req_len, 1); // FORMERR
    }

    size_t offset = 12;
    DNSQuestion q;
    if (!parse_question(req, req_len, offset, q)) {
        return build_error_response(req, req_len, 1);
    }
    if (q.qclass != DNS_CLASS_IN) {
        return build_error_response(req, req_len, 4); // NOTIMP
    }

    std::vector<DNSAnswer> answers;
    bool found = resolve_locally(q.qname, q.qtype, answers);

    // Start building response
    response.reserve(128);
    // ID
    write_u16(response, id);
    // Flags: QR=1, AA=1 (authoritative for our demo), RD copied, RA=0
    uint16_t resp_flags = 0x8000; // QR
    resp_flags |= 0x040; // reserved bit clear? We'll leave AA=0x0400, not 0x040
    // Correct AA bit:
    resp_flags |= 0x0400; // AA
    if (flags & 0x0100) resp_flags |= 0x0100; // RD
    // RCODE
    uint16_t rcode = 0;
    if (!found) rcode = 3; // NXDOMAIN
    resp_flags |= (rcode & 0x000F);
    write_u16(response, resp_flags);
    // QDCOUNT, ANCOUNT, NSCOUNT, ARCOUNT
    write_u16(response, 1);
    write_u16(response, found ? static_cast<uint16_t>(answers.size()) : 0);
    write_u16(response, 0);
    write_u16(response, 0);
    // Write Question
    if (!write_domain_name(response, q.qname)) {
        return build_error_response(req, req_len, 1);
    }
    write_u16(response, q.qtype);
    write_u16(response, q.qclass);

    if (found) {
        for (const auto& a : answers) {
            if (!write_domain_name(response, q.qname)) {
                return build_error_response(req, req_len, 1);
            }
            write_u16(response, a.type);
            write_u16(response, a.cls);
            write_u32(response, a.ttl);
            // RDLENGTH and RDATA
            if (a.rdata.size() > 65535) {
                return build_error_response(req, req_len, 1);
            }
            write_u16(response, static_cast<uint16_t>(a.rdata.size()));
            response.insert(response.end(), a.rdata.begin(), a.rdata.end());
            if (response.size() > DNS_MAX_UDP_SIZE) {
                // Truncate response if exceeds typical 512 bytes; set TC flag if needed (omitted for simplicity)
                break;
            }
        }
    }
    return response;
}

// UDP server: Bind and handle up to max_requests, with a receive timeout
static int run_dns_server(const std::string& bind_ip, uint16_t& inout_port, int max_requests, int timeout_ms) {
    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(inout_port);
    if (::inet_pton(AF_INET, bind_ip.c_str(), &addr.sin_addr) != 1) {
        ::close(fd);
        return -2;
    }
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        perror("bind");
        ::close(fd);
        return -3;
    }
    // If port was 0, get the chosen port
    socklen_t alen = sizeof(addr);
    if (::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &alen) == 0) {
        inout_port = ntohs(addr.sin_port);
    }
    // Set receive timeout
    timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    for (int i = 0; i < max_requests; ++i) {
        uint8_t buf[DNS_MAX_UDP_SIZE];
        sockaddr_in cli{};
        socklen_t clen = sizeof(cli);
        ssize_t n = ::recvfrom(fd, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&cli), &clen);
        if (n < 0) {
            // timeout or error
            break;
        }
        std::vector<uint8_t> resp = handle_dns_request(buf, static_cast<size_t>(n));
        if (!resp.empty()) {
            (void)::sendto(fd, resp.data(), resp.size(), 0, reinterpret_cast<sockaddr*>(&cli), clen);
        }
    }

    ::close(fd);
    return 0;
}

// Helper to craft a minimal DNS query packet (single question)
static std::vector<uint8_t> make_dns_query(uint16_t id, const std::string& qname, uint16_t qtype) {
    std::vector<uint8_t> q;
    q.reserve(64);
    write_u16(q, id);
    // flags: RD=1
    write_u16(q, 0x0100);
    write_u16(q, 1); // QD
    write_u16(q, 0); // AN
    write_u16(q, 0); // NS
    write_u16(q, 0); // AR
    if (!write_domain_name(q, qname)) {
        q.clear();
        return q;
    }
    write_u16(q, qtype);
    write_u16(q, DNS_CLASS_IN);
    return q;
}

// Minimal header parse to check response code and answer count
static bool parse_resp_header(const std::vector<uint8_t>& r, uint16_t& id, uint16_t& rcode, uint16_t& ancount) {
    if (r.size() < 12) return false;
    bool ok = true;
    id = read_u16(r.data(), r.size(), 0, ok);
    if (!ok) return false;
    uint16_t flags = read_u16(r.data(), r.size(), 2, ok);
    if (!ok) return false;
    rcode = flags & 0x000F;
    ancount = read_u16(r.data(), r.size(), 6, ok);
    return ok;
}

int main() {
    try {
        // Test 1: Domain name write and parse round-trip
        {
            std::string name = "Example.COM";
            std::vector<uint8_t> buf;
            if (!write_domain_name(buf, name)) {
                std::cout << "Test1: write failed\n";
            } else {
                size_t off = 0;
                std::string parsed;
                if (!parse_domain_name(buf.data(), buf.size(), off, parsed)) {
                    std::cout << "Test1: parse failed\n";
                } else {
                    std::cout << "Test1: " << parsed << " (len=" << buf.size() << ")\n";
                }
            }
        }

        // Test 2: Local resolver for localhost A
        {
            std::vector<DNSAnswer> ans;
            bool ok = resolve_locally("localhost", DNS_TYPE_A, ans);
            std::cout << "Test2: localhost A resolved=" << (ok ? "yes" : "no") << " answers=" << ans.size() << "\n";
        }

        // Test 3: Handle crafted query for example.com A
        {
            std::vector<uint8_t> q = make_dns_query(0x1234, "example.com", DNS_TYPE_A);
            std::vector<uint8_t> r = handle_dns_request(q.data(), q.size());
            uint16_t id = 0, rcode = 0, an = 0;
            bool ok = parse_resp_header(r, id, rcode, an);
            std::cout << "Test3: ok=" << (ok ? "yes" : "no") << " id=0x" << std::hex << id << std::dec
                      << " rcode=" << rcode << " ancount=" << an << "\n";
        }

        // Start UDP DNS server on 127.0.0.1:0 (OS chooses port), handle 2 requests
        std::promise<uint16_t> p;
        std::future<uint16_t> f = p.get_future();
        std::thread server([&p]() {
            uint16_t port = 0;
            int rc = run_dns_server("127.0.0.1", port, 2, 5000);
            // Communicate the port used (0 if bind failed)
            p.set_value(port);
            (void)rc;
        });

        // Wait briefly for server to bind and retrieve chosen port
        uint16_t srv_port = f.get();
        if (srv_port == 0) {
            std::cout << "Server failed to bind\n";
        } else {
            // Test 4: Send localhost A query to server
            int cfd = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (cfd >= 0) {
                sockaddr_in sa{};
                sa.sin_family = AF_INET;
                sa.sin_port = htons(srv_port);
                inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

                std::vector<uint8_t> q = make_dns_query(0xA001, "localhost", DNS_TYPE_A);
                (void)::sendto(cfd, q.data(), q.size(), 0, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
                uint8_t buf[DNS_MAX_UDP_SIZE];
                sockaddr_in from{};
                socklen_t flen = sizeof(from);
                struct timeval tv;
                tv.tv_sec = 2;
                tv.tv_usec = 0;
                setsockopt(cfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                ssize_t n = ::recvfrom(cfd, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &flen);
                if (n > 0) {
                    std::vector<uint8_t> r(buf, buf + n);
                    uint16_t id = 0, rcode = 0, an = 0;
                    bool ok = parse_resp_header(r, id, rcode, an);
                    std::cout << "Test4: recv=" << n << " ok=" << (ok ? "yes" : "no")
                              << " id=0x" << std::hex << id << std::dec
                              << " rcode=" << rcode << " ancount=" << an << "\n";
                } else {
                    std::cout << "Test4: no response\n";
                }
                ::close(cfd);
            } else {
                std::cout << "Test4: client socket failed\n";
            }

            // Test 5: Send ipv6.local AAAA query to server
            int cfd2 = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (cfd2 >= 0) {
                sockaddr_in sa{};
                sa.sin_family = AF_INET;
                sa.sin_port = htons(srv_port);
                inet_pton(AF_INET, "127.0.0.1", &sa.sin_addr);

                std::vector<uint8_t> q = make_dns_query(0xA002, "ipv6.local", DNS_TYPE_AAAA);
                (void)::sendto(cfd2, q.data(), q.size(), 0, reinterpret_cast<sockaddr*>(&sa), sizeof(sa));
                uint8_t buf[DNS_MAX_UDP_SIZE];
                sockaddr_in from{};
                socklen_t flen = sizeof(from);
                struct timeval tv;
                tv.tv_sec = 2;
                tv.tv_usec = 0;
                setsockopt(cfd2, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                ssize_t n = ::recvfrom(cfd2, buf, sizeof(buf), 0, reinterpret_cast<sockaddr*>(&from), &flen);
                if (n > 0) {
                    std::vector<uint8_t> r(buf, buf + n);
                    uint16_t id = 0, rcode = 0, an = 0;
                    bool ok = parse_resp_header(r, id, rcode, an);
                    std::cout << "Test5: recv=" << n << " ok=" << (ok ? "yes" : "no")
                              << " id=0x" << std::hex << id << std::dec
                              << " rcode=" << rcode << " ancount=" << an << "\n";
                } else {
                    std::cout << "Test5: no response\n";
                }
                ::close(cfd2);
            } else {
                std::cout << "Test5: client socket failed\n";
            }
        }

        if (server.joinable()) server.join();
    } catch (const std::exception& ex) {
        std::cerr << "Unhandled exception: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}