#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/random.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static const size_t MAX_UPLOAD_SIZE = 5 * 1024 * 1024; // 5 MiB
static const size_t MAX_FILENAME_LEN = 100;

static bool sanitizeFilename(const std::string& in, std::string& out) {
    if (in.empty() || in.size() > MAX_FILENAME_LEN) return false;
    for (char c : in) {
        bool ok = (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    if (in == "." || in == "..") return false;
    out = in;
    return true;
}

static std::string hexRandom(size_t nbytes) {
    std::string s;
    s.reserve(nbytes * 2);
    std::vector<unsigned char> buf(nbytes);
    ssize_t r = getrandom(buf.data(), buf.size(), 0);
    if (r < 0 || (size_t)r != buf.size()) {
        // fallback to /dev/urandom
        int fd = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (fd >= 0) {
            size_t off = 0;
            while (off < buf.size()) {
                ssize_t rr = read(fd, buf.data() + off, buf.size() - off);
                if (rr <= 0) { close(fd); break; }
                off += (size_t)rr;
            }
            close(fd);
        } else {
            // worst-case fallback: rand (not cryptographic)
            for (size_t i = 0; i < buf.size(); ++i) buf[i] = (unsigned char)(rand() & 0xFF);
        }
    }
    static const char* hex = "0123456789abcdef";
    for (unsigned char b : buf) {
        s.push_back(hex[(b >> 4) & 0xF]);
        s.push_back(hex[b & 0xF]);
    }
    return s;
}

// Returns 0 on success, non-zero on failure. outSavedName is set to final filename on success.
int upload_file(const std::string& baseDir, const std::string& filename, const std::vector<unsigned char>& data, std::string& outSavedName) {
    if (data.size() > MAX_UPLOAD_SIZE) return -1;
    // Ensure base directory exists
    if (mkdir(baseDir.c_str(), 0700) != 0) {
        if (errno != EEXIST) return -1;
    }
    int dfd = open(baseDir.c_str(), O_DIRECTORY | O_RDONLY | O_CLOEXEC);
    if (dfd < 0) return -1;

    std::string safe;
    if (!sanitizeFilename(filename, safe)) {
        close(dfd);
        return -1;
    }

    // Generate final unique name
    std::string unique = hexRandom(8);
    char tsbuf[32];
    snprintf(tsbuf, sizeof(tsbuf), "%lld", (long long)(time(nullptr) * 1000LL));
    std::string finalName = safe + "-" + unique + "-" + tsbuf;

    // Create temp file
    std::string tmpName = "tmp-" + hexRandom(8) + ".upload";
    int tfd = -1;
    tfd = openat(dfd, tmpName.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (tfd < 0) {
        close(dfd);
        return -1;
    }

    // write data
    size_t off = 0;
    while (off < data.size()) {
        size_t chunk = data.size() - off;
        if (chunk > 65536) chunk = 65536;
        ssize_t w = write(tfd, data.data() + off, chunk);
        if (w <= 0) {
            close(tfd);
            unlinkat(dfd, tmpName.c_str(), 0);
            close(dfd);
            return -1;
        }
        off += (size_t)w;
    }
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(dfd, tmpName.c_str(), 0);
        close(dfd);
        return -1;
    }
    if (close(tfd) != 0) {
        unlinkat(dfd, tmpName.c_str(), 0);
        close(dfd);
        return -1;
    }

    // rename to final name atomically
#if defined(RENAME_NOREPLACE)
    if (renameat2(dfd, tmpName.c_str(), dfd, finalName.c_str(), RENAME_NOREPLACE) != 0)
#else
    if (renameat(dfd, tmpName.c_str(), dfd, finalName.c_str()) != 0)
#endif
    {
        unlinkat(dfd, tmpName.c_str(), 0);
        close(dfd);
        return -1;
    }

    outSavedName = finalName;
    close(dfd);
    return 0;
}

// Minimal HTTP server: POST /upload with headers: Content-Length, X-Filename
static int run_server(const std::string& baseDir, int port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) return -1;
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(srv, (sockaddr*)&addr, sizeof(addr)) != 0) {
        close(srv);
        return -1;
    }
    if (listen(srv, 8) != 0) {
        close(srv);
        return -1;
    }
    std::cout << "C++ server listening on http://127.0.0.1:" << port << "/upload\n";

    while (1) {
        int cfd = accept(srv, nullptr, nullptr);
        if (cfd < 0) continue;

        // Read request header (limit 16KB)
        std::string header;
        header.reserve(16384);
        char buf[1024];
        ssize_t n;
        bool header_done = false;
        size_t total = 0;
        while (!header_done && (n = read(cfd, buf, sizeof(buf))) > 0) {
            header.append(buf, buf + n);
            total += (size_t)n;
            if (total > 16384) break;
            if (header.find("\r\n\r\n") != std::string::npos) {
                header_done = true;
                break;
            }
        }
        if (!header_done) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n";
            write(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        size_t hdr_end = header.find("\r\n\r\n");
        std::string head = header.substr(0, hdr_end + 2); // keep final \r

        std::string rest = header.substr(hdr_end + 4);

        // Parse request line
        std::istringstream hs(head);
        std::string method, path, proto;
        hs >> method >> path >> proto;
        if (method != "POST" || path != "/upload") {
            const char* resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 19\r\nConnection: close\r\n\r\nMethod Not Allowed\n";
            write(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        // Parse headers
        std::string line;
        std::string filename;
        long long content_length = -1;
        while (std::getline(hs, line)) {
            if (!line.empty() && (line.back() == '\r')) line.pop_back();
            auto pos = line.find(':');
            if (pos == std::string::npos) continue;
            std::string key = line.substr(0, pos);
            std::string val = line.substr(pos + 1);
            // trim
            while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());
            for (auto &c : key) c = (char)tolower(c);
            if (key == "content-length") {
                content_length = atoll(val.c_str());
            } else if (key == "x-filename") {
                filename = val;
            }
        }
        if (content_length < 0 || content_length > (long long)MAX_UPLOAD_SIZE || filename.empty()) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n";
            write(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        // Read body: some already in 'rest'
        std::vector<unsigned char> body;
        body.reserve((size_t)content_length);
        body.insert(body.end(), rest.begin(), rest.end());
        while ((long long)body.size() < content_length) {
            n = read(cfd, buf, sizeof(buf));
            if (n <= 0) break;
            size_t to_copy = (size_t)n;
            if ((long long)(body.size() + to_copy) > content_length) {
                to_copy = (size_t)((long long)content_length - (long long)body.size());
            }
            body.insert(body.end(), (unsigned char*)buf, (unsigned char*)buf + to_copy);
        }
        if ((long long)body.size() != content_length) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n";
            write(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        // Trim filename whitespaces
        while (!filename.empty() && (filename.back() == ' ' || filename.back() == '\t')) filename.pop_back();

        std::string saved;
        int rc = upload_file(baseDir, filename, body, saved);
        if (rc == 0) {
            std::string msg = "Saved: " + saved + "\n";
            std::ostringstream oss;
            oss << "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: " << msg.size()
                << "\r\nConnection: close\r\n\r\n" << msg;
            std::string resp = oss.str();
            write(cfd, resp.data(), resp.size());
        } else {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\nConnection: close\r\n\r\nBad Request\n";
            write(cfd, resp, strlen(resp));
        }
        close(cfd);
    }
    close(srv);
    return 0;
}

int main() {
    const std::string base = "uploads-cpp";
    mkdir(base.c_str(), 0700);

    // 5 test cases, calling upload_file directly
    {
        std::vector<unsigned char> d = {'h','e','l','l','o'};
        std::string out;
        if (upload_file(base, "file.txt", d, out) == 0) {
            std::cout << "Test1 OK: " << out << "\n";
        } else {
            std::cout << "Test1 FAIL\n";
        }
    }
    {
        std::vector<unsigned char> d(MAX_UPLOAD_SIZE + 1, 0);
        std::string out;
        if (upload_file(base, "big.bin", d, out) == 0) {
            std::cout << "Test2 FAIL: " << out << "\n";
        } else {
            std::cout << "Test2 OK (rejected large)\n";
        }
    }
    {
        std::vector<unsigned char> d = {1,2,3};
        std::string out;
        if (upload_file(base, "safe-01.log", d, out) == 0) {
            std::cout << "Test3 OK: " << out << "\n";
        } else {
            std::cout << "Test3 FAIL\n";
        }
    }
    {
        std::vector<unsigned char> d = {'x'};
        std::string out;
        if (upload_file(base, "../bad", d, out) == 0) {
            std::cout << "Test4 FAIL: " << out << "\n";
        } else {
            std::cout << "Test4 OK (rejected bad name)\n";
        }
    }
    {
        std::vector<unsigned char> d = {'d','a','t','a'};
        std::string out;
        if (upload_file(base, "bad$name.txt", d, out) == 0) {
            std::cout << "Test5 FAIL: " << out << "\n";
        } else {
            std::cout << "Test5 OK (rejected bad chars)\n";
        }
    }

    // Run HTTP server on port 8081
    run_server(base, 8081);
    return 0;
}