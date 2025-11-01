// Minimal C++ HTTP server for PDF upload to ./uploads
// Note: Designed for POSIX systems. For Windows, additional socket initialization is needed.
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>
#include <filesystem>
#include <fstream>
#include <random>
#include <cctype>
#include <cstring>
#include <cerrno>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "Ws2_32.lib")
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int socket_t;
  #define CLOSESOCK close
#endif

struct Part {
    std::map<std::string, std::string> headers;
    std::vector<unsigned char> data;
};

static const int PORT = 8082;
static const size_t MAX_UPLOAD_SIZE = 10 * 1024 * 1024; // 10MB
static const std::string UPLOAD_DIR = "uploads";

static std::string tolower_str(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}
static std::string sanitize_filename(const std::string& input) {
    std::string name = input;
    for (auto& c : name) if (c == '\\') c = '/';
    auto pos = name.find_last_of('/');
    if (pos != std::string::npos) name = name.substr(pos + 1);
    std::string out;
    for (char c : name) {
        if (std::isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-') out.push_back(c);
        else out.push_back('_');
    }
    if (!out.empty() && out[0] == '.') out[0] = '_';
    if (out.empty()) out = "file.pdf";
    return out;
}
static bool is_pdf_magic(const std::vector<unsigned char>& data) {
    size_t i = 0;
    while (i < data.size() && (data[i] == 0x20 || data[i] == 0x09 || data[i] == 0x0D || data[i] == 0x0A)) i++;
    const char* magic = "%PDF-";
    if (i + 5 > data.size()) return false;
    for (int j = 0; j < 5; ++j) if (data[i + j] != (unsigned char)magic[j]) return false;
    return true;
}
static std::string random_suffix(size_t n=6) {
    static const char* chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> dis(0, 61);
    std::string s;
    for (size_t i = 0; i < n; ++i) s.push_back(chars[dis(gen)]);
    return s;
}
static std::string ensure_unique(const std::string& base) {
    std::filesystem::create_directories(UPLOAD_DIR);
    std::string stem = base, ext;
    std::string lower = tolower_str(base);
    if (lower.size() >= 4 && lower.substr(lower.size()-4) == ".pdf") {
        stem = base.substr(0, base.size() - 4);
        ext = base.substr(base.size() - 4);
    }
    std::string name = base;
    for (int i = 0; i < 1000; ++i) {
        auto p = std::filesystem::path(UPLOAD_DIR) / name;
        if (!std::filesystem::exists(p)) return name;
        name = stem + "-" + random_suffix() + ext;
    }
    return stem + "-" + std::to_string(std::time(nullptr)) + ext;
}
static std::vector<Part> parse_multipart(const std::vector<unsigned char>& body, const std::string& boundary) {
    std::vector<Part> parts;
    std::string delim = "--" + boundary;
    std::string delimCRLF = "\r\n--" + boundary;
    std::string closeDelim = "--" + boundary + "--";
    auto find_bytes = [&](const std::string& needle, size_t from)->ssize_t{
        for (size_t i = from; i + needle.size() <= body.size(); ++i) {
            if (std::memcmp(&body[i], needle.data(), needle.size()) == 0) return (ssize_t)i;
        }
        return -1;
    };
    ssize_t pos = find_bytes(delim, 0);
    if (pos < 0) return parts;
    pos += delim.size();
    if (pos + 2 <= (ssize_t)body.size() && body[pos] == '\r' && body[pos+1] == '\n') pos += 2;

    while (true) {
        // headers
        ssize_t headerEnd = -1;
        for (size_t i = pos; i + 4 <= body.size(); ++i) {
            if (body[i] == '\r' && body[i+1] == '\n' && body[i+2] == '\r' && body[i+3] == '\n') {
                headerEnd = (ssize_t)i;
                break;
            }
        }
        if (headerEnd < 0) break;
        std::string headerBlob((const char*)&body[pos], headerEnd - pos);
        std::map<std::string, std::string> headers;
        size_t start = 0;
        while (start < headerBlob.size()) {
            auto end = headerBlob.find("\r\n", start);
            std::string line = headerBlob.substr(start, (end == std::string::npos ? headerBlob.size() : end) - start);
            auto colon = line.find(':');
            if (colon != std::string::npos) {
                std::string k = tolower_str(std::string(line.begin(), line.begin()+colon));
                std::string v = line.substr(colon+1);
                // trim
                auto l = v.find_first_not_of(" \t");
                auto r = v.find_last_not_of(" \t");
                if (l == std::string::npos) v.clear();
                else v = v.substr(l, r - l + 1);
                headers[k] = v;
            }
            if (end == std::string::npos) break;
            start = end + 2;
        }
        size_t dataStart = headerEnd + 4;
        ssize_t nextBoundary = find_bytes(delimCRLF, dataStart);
        ssize_t nextClose = find_bytes("\r\n" + closeDelim, dataStart);
        bool isLast = false;
        size_t dataEnd;
        if (nextClose != -1 && (nextBoundary == -1 || nextClose < nextBoundary)) {
            dataEnd = (size_t)nextClose;
            isLast = true;
        } else if (nextBoundary != -1) {
            dataEnd = (size_t)nextBoundary;
        } else {
            // try close without CRLF
            ssize_t nb = find_bytes(closeDelim, dataStart);
            if (nb == -1) break;
            dataEnd = (size_t)nb - 2;
            isLast = true;
        }
        if (dataEnd < dataStart) dataEnd = dataStart;

        Part p;
        p.headers = headers;
        p.data.assign(body.begin() + dataStart, body.begin() + dataEnd);
        parts.push_back(std::move(p));

        if (isLast) break;
        pos = (size_t)nextBoundary + 2 + boundary.size();
        if (pos + 2 <= body.size() && body[pos] == '\r' && body[pos+1] == '\n') pos += 2;
    }
    return parts;
}

static void send_response(socket_t client, int code, const std::string& contentType, const std::string& body) {
    std::string status = (code == 200) ? "OK" :
                         (code == 400) ? "Bad Request" :
                         (code == 404) ? "Not Found" :
                         (code == 405) ? "Method Not Allowed" :
                         (code == 413) ? "Payload Too Large" :
                         (code == 415) ? "Unsupported Media Type" :
                         (code == 500) ? "Internal Server Error" : "OK";
    std::string hdr =
        "HTTP/1.1 " + std::to_string(code) + " " + status + "\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "X-Frame-Options: DENY\r\n"
        "Referrer-Policy: no-referrer\r\n"
        "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; form-action 'self'\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Type: " + contentType + "\r\n"
        "Content-Length: " + std::to_string(body.size()) + "\r\n"
        "Connection: close\r\n"
        "\r\n";
    std::string resp = hdr + body;
#ifdef _WIN32
    send(client, resp.c_str(), (int)resp.size(), 0);
#else
    ::send(client, resp.c_str(), resp.size(), 0);
#endif
}

static void handle_client(socket_t client) {
    // Read request (simple, based on Content-Length)
    std::string req;
    char buf[8192];
    int n;
    // Read headers
    while ((n = recv(client, buf, sizeof(buf), MSG_PEEK)) > 0) {
        req.append(buf, buf + n);
        auto pos = req.find("\r\n\r\n");
        if (pos != std::string::npos) {
            // now actually read up to headers end
            size_t to_read = pos + 4;
            size_t total_read = 0;
            while (total_read < to_read) {
                int r = recv(client, buf, (int)std::min(sizeof(buf), to_read - total_read), 0);
                if (r <= 0) break;
                total_read += r;
            }
            break;
        } else {
            // consume what we peeked
            recv(client, buf, n, 0);
        }
    }
    if (req.empty()) {
        CLOSESOCK(client);
        return;
    }

    // Parse start line
    auto line_end = req.find("\r\n");
    if (line_end == std::string::npos) {
        send_response(client, 400, "text/plain; charset=utf-8", "Bad Request");
        CLOSESOCK(client);
        return;
    }
    std::string start = req.substr(0, line_end);
    std::string method, path;
    {
        std::istringstream iss(start);
        iss >> method >> path;
    }

    // Parse headers
    std::map<std::string, std::string> headers;
    size_t pos = line_end + 2;
    size_t header_end = req.find("\r\n\r\n", pos);
    while (pos < header_end) {
        auto e = req.find("\r\n", pos);
        if (e == std::string::npos || e > header_end) break;
        std::string line = req.substr(pos, e - pos);
        auto colon = line.find(':');
        if (colon != std::string::npos) {
            std::string k = line.substr(0, colon);
            std::string v = line.substr(colon + 1);
            // trim
            auto l = v.find_first_not_of(" \t");
            auto r = v.find_last_not_of(" \t");
            if (l == std::string::npos) v.clear();
            else v = v.substr(l, r - l + 1);
            headers[toupper(k[0]) ? tolower_str(k) : k] = v;
        }
        pos = e + 2;
    }

    // Read body if any
    size_t content_length = 0;
    auto it = headers.find("content-length");
    if (it != headers.end()) {
        try { content_length = (size_t)std::stoul(it->second); } catch (...) { content_length = 0; }
    }
    if (content_length > MAX_UPLOAD_SIZE) {
        send_response(client, 413, "text/plain; charset=utf-8", "Payload Too Large");
        CLOSESOCK(client);
        return;
    }

    std::vector<unsigned char> body;
    body.reserve(content_length);
    size_t received = 0;
    while (received < content_length) {
        int r = recv(client, buf, (int)std::min(sizeof(buf), content_length - received), 0);
        if (r <= 0) break;
        body.insert(body.end(), buf, buf + r);
        received += r;
    }

    if (method == "GET" && path == "/") {
        std::string html =
            "<!doctype html><html lang='en'><head><meta charset='utf-8'><title>Upload PDF</title></head>"
            "<body><h1>Upload a PDF</h1>"
            "<form method='POST' action='/upload' enctype='multipart/form-data'>"
            "<input type='file' name='file' accept='application/pdf,.pdf' required>"
            "<button type='submit'>Upload</button>"
            "</form></body></html>";
        send_response(client, 200, "text/html; charset=utf-8", html);
    } else if (method == "POST" && path == "/upload") {
        auto ctit = headers.find("content-type");
        if (ctit == headers.end() || tolower_str(ctit->second).find("multipart/form-data") == std::string::npos) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: expected multipart/form-data");
            CLOSESOCK(client);
            return;
        }
        std::string ctype = ctit->second;
        std::string boundary;
        {
            std::regex re("boundary=([^;]+)");
            std::smatch m;
            if (std::regex_search(ctype, m, re) && m.size() >= 2) {
                boundary = m[1].str();
                if (!boundary.empty() && (boundary.front()=='"' || boundary.front()=='\'')) {
                    boundary = boundary.substr(1, boundary.size()-2);
                }
            }
        }
        if (boundary.empty()) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: missing boundary");
            CLOSESOCK(client);
            return;
        }

        std::vector<Part> parts = parse_multipart(body, boundary);
        Part* filePart = nullptr;
        std::string filename;
        for (auto& p : parts) {
            auto itcd = p.headers.find("content-disposition");
            if (itcd == p.headers.end()) continue;
            std::string cd = itcd->second;
            auto lcd = tolower_str(cd);
            if (lcd.find("name=\"file\"") != std::string::npos && lcd.find("filename=") != std::string::npos) {
                std::regex re("filename=(?:\"([^\"]+)\"|'([^']+)'|([^;]+))", std::regex::icase);
                std::smatch m;
                if (std::regex_search(cd, m, re)) {
                    for (size_t i = 1; i < m.size(); ++i) {
                        if (m[i].matched) { filename = m[i].str(); break; }
                    }
                }
                filePart = &p;
                break;
            }
        }
        if (!filePart || filename.empty()) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: file field missing");
            CLOSESOCK(client);
            return;
        }
        std::string safe = sanitize_filename(filename);
        if (tolower_str(safe).rfind(".pdf") != safe.size()-4) {
            send_response(client, 415, "text/plain; charset=utf-8", "Unsupported Media Type: only .pdf allowed");
            CLOSESOCK(client);
            return;
        }
        if (filePart->data.empty()) {
            send_response(client, 400, "text/plain; charset=utf-8", "Bad Request: empty file");
            CLOSESOCK(client);
            return;
        }
        if (!is_pdf_magic(filePart->data)) {
            send_response(client, 415, "text/plain; charset=utf-8", "Unsupported Media Type: invalid PDF signature");
            CLOSESOCK(client);
            return;
        }
        std::filesystem::create_directories(UPLOAD_DIR);
        std::string unique = ensure_unique(safe);
        std::filesystem::path dest = std::filesystem::path(UPLOAD_DIR) / unique;
        std::ofstream ofs(dest, std::ios::binary | std::ios::out);
        if (!ofs) {
            send_response(client, 500, "text/plain; charset=utf-8", "Server Error: could not save file");
            CLOSESOCK(client);
            return;
        }
        ofs.write(reinterpret_cast<const char*>(filePart->data.data()), (std::streamsize)filePart->data.size());
        ofs.close();
        send_response(client, 200, "text/plain; charset=utf-8", "Uploaded as: " + unique);
    } else {
        send_response(client, 404, "text/plain; charset=utf-8", "Not Found");
    }

    CLOSESOCK(client);
}

static void run_tests() {
    std::cout << "Running C++ tests...\n";
    // 1) sanitize traversal
    auto t1 = sanitize_filename("../etc/passwd");
    if (t1.find("..") != std::string::npos || t1.find('/') != std::string::npos || t1.find('\\') != std::string::npos) {
        std::cerr << "sanitize test 1 failed\n";
        std::exit(1);
    }
    // 2) sanitize hidden
    if (!sanitize_filename(".hidden.pdf").empty() && sanitize_filename(".hidden.pdf")[0] == '.') {
        std::cerr << "sanitize test 2 failed\n";
        std::exit(1);
    }
    // 3) pdf magic true
    {
        std::vector<unsigned char> d = {'%', 'P', 'D', 'F', '-', '1', '.', '7'};
        if (!is_pdf_magic(d)) { std::cerr << "pdf magic test 3 failed\n"; std::exit(1); }
    }
    // 4) pdf magic leading whitespace
    {
        std::vector<unsigned char> d = {'\r','\n',' ','%','P','D','F','-'};
        if (!is_pdf_magic(d)) { std::cerr << "pdf magic test 4 failed\n"; std::exit(1); }
    }
    // 5) pdf magic false
    {
        std::vector<unsigned char> d = {'N','O','T'};
        if (is_pdf_magic(d)) { std::cerr << "pdf magic test 5 failed\n"; std::exit(1); }
    }
    std::cout << "All C++ tests passed.\n";
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    run_tests();

    socket_t server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        std::cerr << "socket error\n";
        return 1;
    }
    int opt = 1;
#ifdef _WIN32
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(server, (sockaddr*)&addr, sizeof(addr)) != 0) {
        std::cerr << "bind error: " << errno << "\n";
        CLOSESOCK(server);
        return 1;
    }
    if (listen(server, 10) != 0) {
        std::cerr << "listen error\n";
        CLOSESOCK(server);
        return 1;
    }
    std::cout << "C++ server running at http://127.0.0.1:" << PORT << "/\n";
    while (true) {
        sockaddr_in caddr{};
        socklen_t clen = sizeof(caddr);
        socket_t client = accept(server, (sockaddr*)&caddr, &clen);
        if (client < 0) continue;
        handle_client(client);
    }
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}