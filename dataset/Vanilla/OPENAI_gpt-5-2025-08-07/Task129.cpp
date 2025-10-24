#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static std::string percent_encode_segment(const std::string& s) {
    std::string out;
    for (unsigned char c : s) {
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back((char)c);
        } else {
            const char* hex = "0123456789ABCDEF";
            out.push_back('%');
            out.push_back(hex[(c >> 4) & 0xF]);
            out.push_back(hex[c & 0xF]);
        }
    }
    return out;
}

static std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static bool has_transfer_encoding_chunked(const std::string& headers) {
    std::string lower = to_lower(headers);
    return lower.find("transfer-encoding: chunked") != std::string::npos;
}

static std::string decode_chunked(const std::string& body) {
    std::string out;
    size_t i = 0;
    while (i < body.size()) {
        // read chunk size line
        size_t line_end = body.find("\r\n", i);
        if (line_end == std::string::npos) break;
        std::string size_str = body.substr(i, line_end - i);
        // strip chunk extensions
        size_t semi = size_str.find(';');
        if (semi != std::string::npos) size_str = size_str.substr(0, semi);
        // parse hex
        size_t chunk_size = 0;
        try {
            chunk_size = std::stoul(size_str, nullptr, 16);
        } catch (...) {
            break;
        }
        i = line_end + 2;
        if (chunk_size == 0) {
            // skip trailing headers if any
            break;
        }
        if (i + chunk_size > body.size()) break;
        out.append(body, i, chunk_size);
        i += chunk_size;
        // skip CRLF after chunk
        if (i + 2 <= body.size() && body.compare(i, 2, "\r\n") == 0) {
            i += 2;
        } else {
            break;
        }
    }
    return out;
}

static std::string build_path(const std::string& prefix, const std::string& filename) {
    std::string p = prefix;
    if (p.empty() || p[0] != '/') p = "/" + p;
    if (!p.empty() && p.size() > 1 && p.back() == '/') {
        // keep
    }
    std::string enc = percent_encode_segment(filename);
    if (enc.empty()) {
        if (p == "/") return "/";
        return p;
    }
    if (p.back() == '/') return p + enc;
    return p + "/" + enc;
}

std::string fetch_file(const std::string& host, const std::string& path_prefix, const std::string& file_name, int port = 80) {
    std::string path = build_path(path_prefix, file_name);

    struct addrinfo hints{};
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    std::string port_str = std::to_string(port);
    int gai = getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res);
    if (gai != 0) {
        return std::string("DNS error: ") + gai_strerror(gai);
    }

    int sockfd = -1;
    for (struct addrinfo* p = res; p; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
        sockfd = -1;
    }
    freeaddrinfo(res);
    if (sockfd == -1) {
        return "Connection error";
    }

    std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nUser-Agent: Task129/1.0\r\nAccept-Encoding: identity\r\nConnection: close\r\n\r\n";
    ssize_t sent = 0;
    while (sent < (ssize_t)req.size()) {
        ssize_t n = send(sockfd, req.data() + sent, req.size() - sent, 0);
        if (n <= 0) { close(sockfd); return "Send error"; }
        sent += n;
    }

    std::string resp;
    char buf[4096];
    for (;;) {
        ssize_t n = recv(sockfd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        resp.append(buf, buf + n);
    }
    close(sockfd);

    size_t psep = resp.find("\r\n\r\n");
    if (psep == std::string::npos) return resp;
    std::string headers = resp.substr(0, psep);
    std::string body = resp.substr(psep + 4);

    if (has_transfer_encoding_chunked(headers)) {
        return decode_chunked(body);
    }
    return body;
}

int main() {
    std::string host = "example.com";
    std::string prefix = "/";
    std::vector<std::string> files = {"", "robots.txt", "favicon.ico", "nonexisting.file", "index.html"};
    for (size_t i = 0; i < files.size(); ++i) {
        std::string content = fetch_file(host, prefix, files[i], 80);
        std::cout << "Test " << (i + 1) << " (" << files[i] << "): length=" << content.size() << "\n";
        std::cout << content.substr(0, std::min<size_t>(content.size(), 200)) << "\n";
        std::cout << "-----\n";
    }
    return 0;
}