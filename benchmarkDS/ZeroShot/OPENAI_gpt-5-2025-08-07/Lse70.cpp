#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <vector>
#include <filesystem>
#include <random>
#include <chrono>
#include <thread>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  typedef SOCKET socket_t;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int socket_t;
#endif

namespace fs = std::filesystem;

static const size_t MAX_UPLOAD_BYTES = 10 * 1024 * 1024;

std::string sanitize_filename(const std::string& name) {
    std::string s;
    s.reserve(name.size());
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') s.push_back(c);
        else s.push_back('_');
    }
    while (!s.empty() && s.front() == '.') s.erase(s.begin());
    if (s.empty()) s = "file";
    if (s.size() > 100) s.resize(100);
    return s;
}

bool upload_file(const std::string& originalName, std::istream& in, size_t maxBytes, std::string& savedPathOut) {
    std::string safe = sanitize_filename(originalName);
    try {
        fs::path uploads = fs::absolute("uploads");
        fs::create_directories(uploads);

        std::string base = safe;
        std::string ext;
        auto pos = safe.find_last_of('.');
        if (pos != std::string::npos && pos > 0 && pos < safe.size() - 1) {
            base = safe.substr(0, pos);
            ext = safe.substr(pos);
        }

        // unique suffix
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<unsigned long long> dist;
        std::string candidate;
        fs::path target;
        for (int i = 0; i < 10; ++i) {
            std::ostringstream oss;
            oss << base << "-" << std::hex << dist(gen) << ext;
            candidate = oss.str();
            target = uploads / candidate;
            if (!fs::exists(target)) break;
        }

        std::ofstream out(target, std::ios::binary | std::ios::out);
        if (!out.is_open()) return false;

        std::vector<char> buf(8192);
        size_t total = 0;
        while (in.good()) {
            in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
            std::streamsize got = in.gcount();
            if (got <= 0) break;
            total += static_cast<size_t>(got);
            if (total > maxBytes) {
                out.close();
                std::error_code ec;
                fs::remove(target, ec);
                return false;
            }
            out.write(buf.data(), got);
        }
        out.close();
        if (total == 0) {
            std::error_code ec;
            fs::remove(target, ec);
            return false;
        }
        savedPathOut = fs::absolute(target).string();
        return true;
    } catch (...) {
        return false;
    }
}

// Minimal HTTP server supporting POST /upload with X-Filename and Content-Length
void http_server_loop(unsigned short port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    socket_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return;

    int opt = 1;
#ifdef _WIN32
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
#endif

    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(srv);
        WSACleanup();
#else
        close(srv);
#endif
        return;
    }
    listen(srv, 5);

    auto send_all = [](socket_t s, const std::string& data) {
        const char* p = data.c_str();
        size_t left = data.size();
        while (left > 0) {
#ifdef _WIN32
            int n = send(s, p, (int)left, 0);
#else
            ssize_t n = send(s, p, left, 0);
#endif
            if (n <= 0) break;
            p += n;
            left -= n;
        }
    };

    for (;;) {
        sockaddr_in cli{};
#ifdef _WIN32
        int clen = sizeof(cli);
        socket_t c = accept(srv, (sockaddr*)&cli, &clen);
#else
        socklen_t clen = sizeof(cli);
        socket_t c = accept(srv, (sockaddr*)&cli, &clen);
#endif
        if (c < 0) continue;

        // Read headers
        std::string headers;
        char ch;
        int crlf_count = 0;
        while (true) {
#ifdef _WIN32
            int n = recv(c, &ch, 1, 0);
#else
            ssize_t n = recv(c, &ch, 1, 0);
#endif
            if (n <= 0) break;
            headers.push_back(ch);
            size_t N = headers.size();
            if (N >= 4 && headers[N-4] == '\r' && headers[N-3] == '\n' && headers[N-2] == '\r' && headers[N-1] == '\n') {
                break;
            }
            if (headers.size() > 64 * 1024) break; // header limit
        }

        auto bad_request = [&](const std::string& msg, int code) {
            std::ostringstream oss;
            oss << "HTTP/1.1 " << code << " Error\r\n"
                << "X-Content-Type-Options: nosniff\r\n"
                << "X-Frame-Options: DENY\r\n"
                << "Content-Security-Policy: default-src 'none'\r\n"
                << "Cache-Control: no-store\r\n"
                << "Content-Type: text/plain; charset=utf-8\r\n"
                << "Content-Length: " << msg.size() << "\r\n\r\n" << msg;
            send_all(c, oss.str());
        };

        if (headers.find("HTTP/") == std::string::npos) {
            bad_request("Malformed request", 400);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }

        std::istringstream hs(headers);
        std::string method, path, version;
        hs >> method >> path >> version;
        if (method != "POST" || path != "/upload") {
            bad_request("Not Found", 404);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }

        std::string line;
        std::string xfilename;
        long long content_length = -1;
        while (std::getline(hs, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) break;
            auto posc = line.find(':');
            if (posc != std::string::npos) {
                std::string key = line.substr(0, posc);
                std::string val = line.substr(posc + 1);
                // trim
                auto ltrim = [](std::string& s){ s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int c){return !std::isspace(c);})); };
                auto rtrim = [](std::string& s){ s.erase(std::find_if(s.rbegin(), s.rend(), [](int c){return !std::isspace(c);}).base(), s.end()); };
                ltrim(val); rtrim(val);
                for (auto& cch : key) cch = (char)std::tolower((unsigned char)cch);
                if (key == "x-filename") xfilename = val;
                if (key == "content-length") {
                    try { content_length = std::stoll(val); } catch (...) { content_length = -1; }
                }
            }
        }

        if (xfilename.empty()) {
            bad_request("Missing X-Filename header", 400);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }
        if (content_length <= 0) {
            bad_request("Content-Length required", 411);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }
        if (content_length > (long long)MAX_UPLOAD_BYTES) {
            bad_request("Payload too large", 413);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }

        std::vector<char> body;
        body.resize((size_t)content_length);
        size_t got = 0;
        while (got < (size_t)content_length) {
#ifdef _WIN32
            int n = recv(c, body.data() + got, (int)((size_t)content_length - got), 0);
#else
            ssize_t n = recv(c, body.data() + got, (size_t)content_length - got, 0);
#endif
            if (n <= 0) break;
            got += (size_t)n;
        }
        if (got != (size_t)content_length) {
            bad_request("Incomplete body", 400);
#ifdef _WIN32
            closesocket(c);
#else
            close(c);
#endif
            continue;
        }

        std::string saved;
        std::istringstream in(std::string(body.begin(), body.end()));
        bool ok = upload_file(xfilename, in, MAX_UPLOAD_BYTES, saved);

        if (!ok) {
            bad_request("Upload failed", 400);
        } else {
            std::string msg = "Saved: " + saved;
            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\n"
                << "X-Content-Type-Options: nosniff\r\n"
                << "X-Frame-Options: DENY\r\n"
                << "Content-Security-Policy: default-src 'none'\r\n"
                << "Cache-Control: no-store\r\n"
                << "Content-Type: text/plain; charset=utf-8\r\n"
                << "Content-Length: " << msg.size() << "\r\n\r\n" << msg;
            send_all(c, oss.str());
        }

#ifdef _WIN32
        closesocket(c);
#else
        close(c);
#endif
    }

#ifdef _WIN32
    closesocket(srv);
    WSACleanup();
#else
    close(srv);
#endif
}

int main() {
    // Start HTTP server in a background thread
    std::thread srv([](){ http_server_loop(8082); });
    srv.detach();

    std::cout << "Running tests for upload_file..." << std::endl;

    // Test 1
    {
        std::istringstream in("Hello world");
        std::string saved;
        if (upload_file("hello.txt", in, 1024, saved)) {
            std::cout << "Test 1 saved: " << saved << std::endl;
        } else {
            std::cout << "Test 1 failed" << std::endl;
        }
    }

    // Test 2
    {
        std::istringstream in("evil content");
        std::string saved;
        if (upload_file("../evil.sh", in, 1024, saved)) {
            std::cout << "Test 2 saved: " << saved << std::endl;
        } else {
            std::cout << "Test 2 failed" << std::endl;
        }
    }

    // Test 3
    {
        std::istringstream in("data");
        std::string saved;
        if (upload_file("inv@lid:/name?.bin", in, 1024, saved)) {
            std::cout << "Test 3 saved: " << saved << std::endl;
        } else {
            std::cout << "Test 3 failed" << std::endl;
        }
    }

    // Test 4
    {
        std::istringstream in("bashrc-like");
        std::string saved;
        if (upload_file(".bashrc", in, 1024, saved)) {
            std::cout << "Test 4 saved: " << saved << std::endl;
        } else {
            std::cout << "Test 4 failed" << std::endl;
        }
    }

    // Test 5
    {
        std::string big(2 * 1024 * 1024, '\0');
        std::istringstream in(big);
        std::string saved;
        if (upload_file("toolarge.dat", in, 1024 * 1024, saved)) {
            std::cout << "Test 5 unexpectedly succeeded: " << saved << std::endl;
        } else {
            std::cout << "Test 5 correctly failed" << std::endl;
        }
    }

    std::cout << "HTTP server listening on http://127.0.0.1:8082/upload" << std::endl;
    // Keep running
    std::this_thread::sleep_for(std::chrono::hours(24));
    return 0;
}