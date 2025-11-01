#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

namespace fs = std::filesystem;

static void ensure_uploads_dir() {
    fs::create_directories("uploads");
}

static std::string sanitize_filename(const std::string& name) {
    std::string base = name;
    std::replace(base.begin(), base.end(), '\\', '/');
    auto pos = base.find_last_of('/');
    if (pos != std::string::npos) base = base.substr(pos + 1);
    // remove '..'
    std::string out;
    out.reserve(base.size());
    for (size_t i = 0; i < base.size(); ++i) {
        if (i + 1 < base.size() && base[i] == '.' && base[i + 1] == '.') {
            // skip
        } else {
            out.push_back(base[i]);
        }
    }
    if (out.empty()) out = "upload.bin";
    return out;
}

static std::string url_decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '%' && i + 2 < s.size()) {
            char hex[3] = { s[i+1], s[i+2], 0 };
            char* endp = nullptr;
            long v = strtol(hex, &endp, 16);
            if (*endp == 0) {
                out.push_back(static_cast<char>(v));
                i += 2;
            } else {
                out.push_back(s[i]);
            }
        } else if (s[i] == '+') {
            out.push_back(' ');
        } else {
            out.push_back(s[i]);
        }
    }
    return out;
}

// The upload_file function: saves data under uploads directory. Returns absolute path string.
std::string upload_file(const std::string& filename, const std::vector<unsigned char>& data) {
    ensure_uploads_dir();
    std::string safe = sanitize_filename(filename);
    fs::path out = fs::path("uploads") / safe;
    std::ofstream ofs(out, std::ios::binary | std::ios::trunc);
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    ofs.close();
    return fs::absolute(out).string();
}

static void handle_client(int client_fd) {
    std::string header;
    char buf[4096];
    // Read until we find \r
\r

    while (header.find("\r\n\r\n") == std::string::npos) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) { close(client_fd); return; }
        header.append(buf, buf + n);
        if (header.size() > 1 << 20) { close(client_fd); return; } // safety: 1MB
    }

    // Split headers and maybe some body
    size_t header_end = header.find("\r\n\r\n");
    std::string head = header.substr(0, header_end);
    std::string remain = header.substr(header_end + 4);

    // Parse request line
    size_t line_end = head.find("\r\n");
    if (line_end == std::string::npos) { close(client_fd); return; }
    std::string request_line = head.substr(0, line_end);
    // Expect: POST /upload?filename=... HTTP/1.1
    std::string method, path, version;
    {
        size_t p1 = request_line.find(' ');
        size_t p2 = p1 == std::string::npos ? std::string::npos : request_line.find(' ', p1 + 1);
        if (p1 == std::string::npos || p2 == std::string::npos) { close(client_fd); return; }
        method = request_line.substr(0, p1);
        path = request_line.substr(p1 + 1, p2 - (p1 + 1));
        version = request_line.substr(p2 + 1);
    }
    if (method != "POST") {
        std::string resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        close(client_fd);
        return;
    }

    // Content-Length
    size_t cl_pos = head.find("Content-Length:");
    if (cl_pos == std::string::npos) cl_pos = head.find("content-length:");
    if (cl_pos == std::string::npos) {
        std::string resp = "HTTP/1.1 411 Length Required\r\nContent-Length: 0\r\n\r\n";
        send(client_fd, resp.c_str(), resp.size(), 0);
        close(client_fd);
        return;
    }
    size_t cl_end = head.find("\r\n", cl_pos);
    std::string cl_line = head.substr(cl_pos, cl_end - cl_pos);
    size_t colon = cl_line.find(':');
    size_t start = colon == std::string::npos ? std::string::npos : cl_line.find_first_not_of(" \t", colon + 1);
    long content_length = 0;
    if (start != std::string::npos) {
        content_length = std::stol(cl_line.substr(start));
    }

    // Parse filename from query
    std::string filename;
    {
        size_t qpos = path.find('?');
        if (qpos != std::string::npos) {
            std::string query = path.substr(qpos + 1);
            // find filename=
            std::string key = "filename=";
            size_t kpos = query.find(key);
            if (kpos != std::string::npos) {
                size_t vstart = kpos + key.size();
                size_t vend = query.find('&', vstart);
                filename = query.substr(vstart, vend == std::string::npos ? std::string::npos : vend - vstart);
                filename = url_decode(filename);
            }
        }
    }
    if (filename.empty()) {
        std::string resp = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 29\r\n\r\nMissing filename parameter";
        send(client_fd, resp.c_str(), resp.size(), 0);
        close(client_fd);
        return;
    }

    std::string body = remain;
    while ((long)body.size() < content_length) {
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        body.append(buf, buf + n);
    }
    if ((long)body.size() > content_length) {
        body.resize((size_t)content_length);
    }

    std::vector<unsigned char> data(body.begin(), body.end());
    std::string saved = upload_file(filename, data);
    std::string msg = "Saved: " + saved + "\n";
    std::string resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: " + std::to_string(msg.size()) + "\r\n\r\n" + msg;
    send(client_fd, resp.c_str(), resp.size(), 0);
    close(client_fd);
}

static void start_server(unsigned short port) {
    std::thread([port]() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) return;
        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);
        if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) != 0) {
            close(server_fd);
            return;
        }
        if (listen(server_fd, 16) != 0) {
            close(server_fd);
            return;
        }
        while (true) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) break;
            std::thread(handle_client, client_fd).detach();
        }
        close(server_fd);
    }).detach();
}

int main() {
    // Start server on port 8082
    start_server(8082);

    // 5 test cases calling upload_file directly
    std::cout << upload_file("hello.txt", std::vector<unsigned char>{'H','e','l','l','o',',',' ','C','+','+','!'} ) << std::endl;
    std::cout << upload_file("data.bin", std::vector<unsigned char>{0,1,2,3,4,5}) << std::endl;
    std::cout << upload_file("image.jpg", std::vector<unsigned char>{0xFF, 0xD8, 0x00, 0x01}) << std::endl;
    std::cout << upload_file("../evil/../../should_not_escape.txt", std::vector<unsigned char>{'s','a','f','e'}) << std::endl;
    std::string uni = "Привет";
    std::cout << upload_file("unicode_файл.txt", std::vector<unsigned char>(uni.begin(), uni.end())) << std::endl;

    return 0;
}