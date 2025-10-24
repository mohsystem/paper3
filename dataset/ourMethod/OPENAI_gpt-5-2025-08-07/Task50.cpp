#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

static const size_t MAX_HEADER_SIZE = 16384;   // 16 KiB
static const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MiB
static const int SOCKET_TIMEOUT_SEC = 10;

struct ServerConfig {
    std::string bindAddress = "127.0.0.1";
    uint16_t port = 9090;
    std::string baseDir = "uploads_cpp";
    int maxRequests = 5;
};

static std::string sanitizeFilename(const std::string &in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    while (!out.empty() && (out[0] == '.' || out[0] == '_')) {
        out.erase(out.begin());
    }
    if (out.empty()) out = "file";
    if (out.size() > 100) out.resize(100);
    return out;
}

static bool createSecureDir(const std::string &path) {
    try {
        if (!fs::exists(path)) {
            fs::create_directories(path);
        }
        fs::permissions(path, fs::perms::owner_all, fs::perm_options::replace);
        return fs::is_directory(path);
    } catch (...) {
        return false;
    }
}

static std::string randomHex(size_t bytes) {
    std::random_device rd;
    std::vector<unsigned char> buf(bytes);
    for (size_t i = 0; i < bytes; ++i) {
        buf[i] = static_cast<unsigned char>(rd());
    }
    static const char *hex = "0123456789abcdef";
    std::string out;
    out.reserve(bytes * 2);
    for (unsigned char b : buf) {
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

static bool setSocketTimeouts(int fd, int seconds) {
    timeval tv{};
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) return false;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) return false;
    return true;
}

static bool readUntil(int fd, std::string &buf, const std::string &delim, size_t maxBytes) {
    char temp[4096];
    while (buf.find(delim) == std::string::npos) {
        ssize_t n = recv(fd, temp, sizeof(temp), 0);
        if (n == 0) return false; // closed
        if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        buf.append(temp, temp + n);
        if (buf.size() > maxBytes) return false;
    }
    return true;
}

static std::map<std::string, std::string> parseHeaders(const std::string &headers) {
    std::map<std::string, std::string> m;
    std::istringstream iss(headers);
    std::string line;
    bool first = true;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (first) {
            m[":request-line"] = line;
            first = false;
            continue;
        }
        if (line.empty()) break;
        auto pos = line.find(':');
        if (pos == std::string::npos) continue;
        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);
        // trim spaces
        while (!val.empty() && (val.front() == ' ' || val.front() == '\t')) val.erase(val.begin());
        while (!val.empty() && (val.back() == ' ' || val.back() == '\t')) val.pop_back();
        for (auto &c : key) c = static_cast<char>(::tolower(c));
        m[key] = val;
    }
    return m;
}

static std::string buildHttpResponse(int code, const std::string &body) {
    std::ostringstream oss;
    std::string status = (code == 200) ? "OK" : (code == 400 ? "Bad Request" : (code == 413 ? "Payload Too Large" : "Internal Server Error"));
    oss << "HTTP/1.1 " << code << " " << status << "\r\n";
    oss << "Content-Type: text/plain; charset=utf-8\r\n";
    oss << "Content-Length: " << body.size() << "\r\n";
    oss << "Connection: close\r\n\r\n";
    oss << body;
    return oss.str();
}

static bool atomicWriteAndMove(const std::string &baseDir, const std::string &finalName, int clientFd, size_t contentLen, std::string &finalPath, std::string &err, const std::string &initialBody) {
    // Construct temp template in same directory
    std::string tmpl = baseDir + "/upload-XXXXXX.tmp";
    std::vector<char> tempPath(tmpl.begin(), tmpl.end());
    tempPath.push_back('\0');
    int tmpFd = mkstemp(tempPath.data());
    if (tmpFd < 0) {
        err = "Failed to create temp file";
        return false;
    }
    // Set permissions 0600
    fchmod(tmpFd, S_IRUSR | S_IWUSR);

    // Write body: initialBody already read after headers
    size_t remaining = contentLen;
    size_t wrote = 0;

    // First write from initial body chunk
    if (!initialBody.empty()) {
        size_t chunk = std::min(initialBody.size(), remaining);
        ssize_t wn = write(tmpFd, initialBody.data(), chunk);
        if (wn < 0 || static_cast<size_t>(wn) != chunk) {
            err = "Write failed";
            close(tmpFd);
            unlink(tempPath.data());
            return false;
        }
        remaining -= chunk;
        wrote += chunk;
    }

    char buf[8192];
    while (remaining > 0) {
        size_t toRead = std::min(remaining, sizeof(buf));
        ssize_t rn = recv(clientFd, buf, toRead, 0);
        if (rn <= 0) {
            err = "Socket read failed or closed";
            close(tmpFd);
            unlink(tempPath.data());
            return false;
        }
        size_t off = 0;
        while (off < static_cast<size_t>(rn)) {
            ssize_t wn = write(tmpFd, buf + off, rn - off);
            if (wn <= 0) {
                err = "Write failed";
                close(tmpFd);
                unlink(tempPath.data());
                return false;
            }
            off += static_cast<size_t>(wn);
        }
        remaining -= static_cast<size_t>(rn);
        wrote += static_cast<size_t>(rn);
        if (wrote > MAX_FILE_SIZE) {
            err = "Exceeded max file size while writing";
            close(tmpFd);
            unlink(tempPath.data());
            return false;
        }
    }

    // Flush and fsync
    if (fsync(tmpFd) != 0) {
        err = "fsync failed";
        close(tmpFd);
        unlink(tempPath.data());
        return false;
    }
    if (close(tmpFd) != 0) {
        err = "close failed";
        unlink(tempPath.data());
        return false;
    }

    // Unique final path with suffix to avoid collisions
    std::string unique = finalName + "_" + randomHex(6);
    finalPath = baseDir + "/" + unique;

    // Attempt rename; retry a couple of times on EEXIST
    for (int i = 0; i < 3; ++i) {
        if (rename(tempPath.data(), finalPath.c_str()) == 0) {
            return true;
        }
        if (errno == EEXIST) {
            unique = finalName + "_" + randomHex(6);
            finalPath = baseDir + "/" + unique;
            continue;
        }
        break;
    }
    err = "rename failed";
    unlink(tempPath.data());
    return false;
}

static void handleClient(int cfd, const ServerConfig &cfg, int &processedCount) {
    setSocketTimeouts(cfd, SOCKET_TIMEOUT_SEC);
    std::string data;
    data.reserve(4096);
    if (!readUntil(cfd, data, "\r\n\r\n", MAX_HEADER_SIZE)) {
        std::string resp = buildHttpResponse(400, "Invalid HTTP request\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }
    size_t pos = data.find("\r\n\r\n");
    std::string headerText = data.substr(0, pos + 2); // include last CRLF of headers lines
    std::string initialBody = data.substr(pos + 4);

    auto headers = parseHeaders(headerText);
    auto itReq = headers.find(":request-line");
    if (itReq == headers.end()) {
        std::string resp = buildHttpResponse(400, "Bad request line\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }

    // Parse request line: METHOD PATH HTTP/1.1
    std::istringstream rl(itReq->second);
    std::string method, path, version;
    rl >> method >> path >> version;
    if (method != "POST" || path != "/upload") {
        std::string resp = buildHttpResponse(400, "Only POST /upload allowed\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }

    // Filename header
    std::string filename;
    auto itFn = headers.find("x-filename");
    if (itFn != headers.end()) filename = itFn->second;
    if (filename.empty()) {
        auto itFn2 = headers.find("filename");
        if (itFn2 != headers.end()) filename = itFn2->second;
    }
    if (filename.empty()) {
        std::string resp = buildHttpResponse(400, "Missing X-Filename header\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }
    filename = sanitizeFilename(filename);

    // Content-Length
    auto itCL = headers.find("content-length");
    if (itCL == headers.end()) {
        std::string resp = buildHttpResponse(400, "Missing Content-Length\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }
    size_t contentLen = 0;
    {
        try {
            size_t idx = 0;
            unsigned long long val = std::stoull(itCL->second, &idx, 10);
            if (idx != itCL->second.size()) throw std::runtime_error("non-numeric");
            if (val > MAX_FILE_SIZE) {
                std::string resp = buildHttpResponse(413, "File too large\n");
                send(cfd, resp.data(), resp.size(), 0);
                close(cfd);
                return;
            }
            contentLen = static_cast<size_t>(val);
        } catch (...) {
            std::string resp = buildHttpResponse(400, "Invalid Content-Length\n");
            send(cfd, resp.data(), resp.size(), 0);
            close(cfd);
            return;
        }
    }

    if (!createSecureDir(cfg.baseDir)) {
        std::string resp = buildHttpResponse(500, "Server storage error\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }

    std::string finalPath;
    std::string err;
    if (!atomicWriteAndMove(cfg.baseDir, filename, cfd, contentLen, finalPath, err, initialBody)) {
        std::string resp = buildHttpResponse(500, "Failed to store file: " + err + "\n");
        send(cfd, resp.data(), resp.size(), 0);
        close(cfd);
        return;
    }

    // Respond success
    std::string resp = buildHttpResponse(200, "Upload successful: " + fs::path(finalPath).filename().string() + "\n");
    send(cfd, resp.data(), resp.size(), 0);
    close(cfd);
    processedCount++;
}

static void runServer(const ServerConfig &cfg) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        std::cerr << "Failed to create socket\n";
        return;
    }
    int yes = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg.port);
    inet_pton(AF_INET, cfg.bindAddress.c_str(), &addr.sin_addr);

    if (bind(sfd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        std::cerr << "Bind failed: " << strerror(errno) << "\n";
        close(sfd);
        return;
    }
    if (listen(sfd, 16) != 0) {
        std::cerr << "Listen failed\n";
        close(sfd);
        return;
    }

    int processed = 0;
    while (processed < cfg.maxRequests) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        int cfd = accept(sfd, (sockaddr *)&caddr, &clen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            break;
        }
        handleClient(cfd, cfg, processed);
    }
    close(sfd);
}

static std::string sendUpload(const std::string &host, uint16_t port, const std::string &filename, const std::string &content) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return "socket error";
    setSocketTimeouts(fd, SOCKET_TIMEOUT_SEC);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {
        close(fd);
        return "inet_pton error";
    }
    if (connect(fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        close(fd);
        return "connect error";
    }
    std::ostringstream req;
    req << "POST /upload HTTP/1.1\r\n";
    req << "Host: " << host << "\r\n";
    req << "Content-Length: " << content.size() << "\r\n";
    req << "X-Filename: " << filename << "\r\n";
    req << "Connection: close\r\n\r\n";
    std::string head = req.str();

    // send headers
    ssize_t wn = send(fd, head.data(), head.size(), 0);
    if (wn < 0 || static_cast<size_t>(wn) != head.size()) {
        close(fd);
        return "send error";
    }
    // send body
    size_t off = 0;
    while (off < content.size()) {
        ssize_t n = send(fd, content.data() + off, content.size() - off, 0);
        if (n <= 0) {
            close(fd);
            return "send body error";
        }
        off += static_cast<size_t>(n);
    }

    // read response
    std::string resp;
    char buf[2048];
    for (;;) {
        ssize_t rn = recv(fd, buf, sizeof(buf), 0);
        if (rn < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (rn == 0) break;
        resp.append(buf, buf + rn);
        if (resp.size() > 65536) break;
    }
    close(fd);
    return resp;
}

int main() {
    ServerConfig cfg;
    cfg.port = 9090;
    cfg.baseDir = "uploads_cpp";
    cfg.maxRequests = 5;

    std::thread serverThread([&cfg]() { runServer(cfg); });

    // Give server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // 5 test uploads
    std::vector<std::pair<std::string, std::string>> tests = {
        {"report1.txt", "Hello World 1"},
        {"safe_name-02.log", std::string(1024, 'A')}, // 1KB
        {"../tricky/evil.bin", "binary\x00data"},     // sanitized
        {"..hidden", "no longer hidden"},
        {"longname______________________________________________________________with*bad?chars.txt", "small"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string resp = sendUpload("127.0.0.1", cfg.port, tests[i].first, tests[i].second);
        // Print first line of response
        auto pos = resp.find("\r\n");
        std::string status = (pos != std::string::npos) ? resp.substr(0, pos) : resp;
        std::cout << "Test " << (i + 1) << ": " << status << "\n";
        auto bodyPos = resp.find("\r\n\r\n");
        if (bodyPos != std::string::npos) {
            std::cout << resp.substr(bodyPos + 4);
        }
    }

    serverThread.join();
    return 0;
}