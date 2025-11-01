#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static bool is_regular_mode(mode_t m) {
    return S_ISREG(m);
}

static std::string safe_filename(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == '.' || c == '_' || c == '-' || c == ' ') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    return out;
}

static std::string gen_boundary() {
    // Simple random-ish boundary
    unsigned int seed = (unsigned int) ::getpid() ^ (unsigned int) ::time(nullptr);
    std::ostringstream oss;
    oss << "----cppboundary";
    for (int i = 0; i < 8; ++i) {
        seed = 1664525u * seed + 1013904223u;
        unsigned int v = seed;
        const char* hex = "0123456789abcdef";
        oss << hex[(v >> 28) & 0xF] << hex[(v >> 24) & 0xF] << hex[(v >> 20) & 0xF] << hex[(v >> 16) & 0xF]
            << hex[(v >> 12) & 0xF] << hex[(v >> 8) & 0xF] << hex[(v >> 4) & 0xF] << hex[v & 0xF];
    }
    return oss.str();
}

static bool read_file_secure(const std::string& path, std::vector<unsigned char>& data, size_t max_size, std::string& err) {
    if (path.size() < 4 || path.substr(path.size() - 4) != ".pdf") {
        err = "Invalid extension";
        return false;
    }
    int fd = ::open(path.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        if (errno == ELOOP) err = "Symlink not allowed";
        else err = "Open failed";
        return false;
    }
    struct stat st;
    if (::fstat(fd, &st) != 0) {
        err = "fstat failed";
        ::close(fd);
        return false;
    }
    if (!is_regular_mode(st.st_mode)) {
        err = "Not a regular file";
        ::close(fd);
        return false;
    }
    if (st.st_size <= 0 || (size_t)st.st_size > max_size) {
        err = "Invalid size";
        ::close(fd);
        return false;
    }
    data.resize((size_t)st.st_size);
    size_t off = 0;
    while (off < data.size()) {
        ssize_t r = ::read(fd, data.data() + off, std::min<size_t>(65536, data.size() - off));
        if (r < 0) {
            err = "Read error";
            ::close(fd);
            return false;
        }
        if (r == 0) break;
        off += (size_t)r;
    }
    ::close(fd);
    if (off != data.size()) {
        err = "Short read";
        return false;
    }
    if (data.size() < 5 || !(data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-')) {
        err = "Invalid PDF signature";
        return false;
    }
    return true;
}

static int upload_pdf_to_localhost(const std::string& path, const std::string& endpoint, size_t max_size, int port, std::string& response) {
    std::vector<unsigned char> content;
    std::string err;
    if (!read_file_secure(path, content, max_size, err)) {
        response = err;
        return -1;
    }
    std::string ep = endpoint.size() > 0 && endpoint[0] == '/' ? endpoint : "/upload";
    std::string host = "127.0.0.1";

    std::string boundary = gen_boundary();
    std::string filename = path.substr(path.find_last_of("/\\") == std::string::npos ? 0 : path.find_last_of("/\\") + 1);
    std::ostringstream hdr;
    hdr << "--" << boundary << "\r\n";
    hdr << "Content-Disposition: form-data; name=\"file\"; filename=\"" << safe_filename(filename) << "\"\r\n";
    hdr << "Content-Type: application/pdf\r\n\r\n";
    std::string header = hdr.str();
    std::string footer = std::string("\r\n--") + boundary + "--\r\n";

    size_t total_len = header.size() + content.size() + footer.size();

    // Build request head with Content-Length
    std::ostringstream reqhead;
    reqhead << "POST " << ep << " HTTP/1.1\r\n";
    reqhead << "Host: " << host << ":" << port << "\r\n";
    reqhead << "Connection: close\r\n";
    reqhead << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";
    reqhead << "Content-Length: " << total_len << "\r\n\r\n";
    std::string head = reqhead.str();

    // Connect to localhost
    int s = ::socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        response = "Socket error";
        return -1;
    }
    struct timeval tv;
    tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    if (::inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        ::close(s);
        response = "inet_pton failed";
        return -1;
    }
    if (::connect(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        ::close(s);
        response = "Connect failed";
        return -1;
    }

    auto send_all = [&](const void* buf, size_t len)->bool {
        const unsigned char* p = (const unsigned char*)buf;
        size_t off = 0;
        while (off < len) {
            ssize_t w = ::send(s, p + off, len - off, 0);
            if (w <= 0) return false;
            off += (size_t)w;
        }
        return true;
    };

    bool ok = send_all(head.data(), head.size())
           && send_all(header.data(), header.size())
           && send_all(content.data(), content.size())
           && send_all(footer.data(), footer.size());
    if (!ok) {
        ::close(s);
        response = "Send failed";
        return -1;
    }

    // Read response
    std::string resp;
    char buf[4096];
    for (;;) {
        ssize_t r = ::recv(s, buf, sizeof(buf), 0);
        if (r < 0) {
            ::close(s);
            response = "Recv failed";
            return -1;
        }
        if (r == 0) break;
        resp.append(buf, buf + r);
    }
    ::close(s);
    response = resp;

    // Parse status
    int status = -1;
    size_t sp1 = resp.find(' ');
    if (sp1 != std::string::npos && resp.size() >= sp1 + 4) {
        status = std::atoi(resp.substr(sp1 + 1, 3).c_str());
    }
    return status;
}

static bool write_file_secure(const std::string& path, const std::vector<unsigned char>& data) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return false;
    size_t off = 0;
    while (off < data.size()) {
        ssize_t w = ::write(fd, data.data() + off, std::min<size_t>(65536, data.size() - off));
        if (w <= 0) {
            ::close(fd);
            return false;
        }
        off += (size_t)w;
    }
    ::fsync(fd);
    ::close(fd);
    return true;
}

int main() {
    const size_t max_size = 512 * 1024;
    std::string dir = "/tmp/cpp_pdfup_XXXXXX";
    std::vector<char> dirbuf(dir.begin(), dir.end());
    dirbuf.push_back('\0');
    char* d = ::mkdtemp(dirbuf.data());
    if (!d) {
        std::cerr << "Failed to create temp dir\n";
        return 1;
    }
    std::string base(d);

    // Build test files
    std::vector<unsigned char> minimal_pdf;
    {
        const char* s = "%PDF-1.4\n1 0 obj\n<<>>\nendobj\ntrailer\n<<>>\n%%EOF\n";
        minimal_pdf.assign(s, s + std::strlen(s));
    }
    std::string f1 = base + "/t1.pdf";
    std::string f2 = base + "/t2.pdf";
    std::string f3 = base + "/notes.txt";
    std::string f4 = base + "/bad.pdf";
    std::string f5 = base + "/big.pdf";

    write_file_secure(f1, minimal_pdf);
    write_file_secure(f2, minimal_pdf);
    write_file_secure(f3, minimal_pdf);
    {
        const char* bad = "HELLO";
        std::vector<unsigned char> v(bad, bad + std::strlen(bad));
        write_file_secure(f4, v);
    }
    {
        std::vector<unsigned char> big;
        const char* head = "%PDF-1.4\n";
        big.insert(big.end(), head, head + std::strlen(head));
        std::vector<unsigned char> chunk(64 * 1024, 'A');
        size_t repeats = (max_size / chunk.size()) + 2;
        for (size_t i = 0; i < repeats; ++i) big.insert(big.end(), chunk.begin(), chunk.end());
        const char* eof = "%%EOF\n";
        big.insert(big.end(), eof, eof + std::strlen(eof));
        write_file_secure(f5, big);
    }

    std::string resp;
    int s1 = upload_pdf_to_localhost(f1, "/upload", max_size, 8000, resp);
    std::cout << "Test1: status=" << s1 << " resp_len=" << resp.size() << "\n";
    int s2 = upload_pdf_to_localhost(f2, "/upload", max_size, 8000, resp);
    std::cout << "Test2: status=" << s2 << " resp_len=" << resp.size() << "\n";
    int s3 = upload_pdf_to_localhost(f3, "/upload", max_size, 8000, resp);
    std::cout << "Test3: status=" << s3 << " err='" << resp << "'\n";
    int s4 = upload_pdf_to_localhost(f4, "/upload", max_size, 8000, resp);
    std::cout << "Test4: status=" << s4 << " err='" << resp << "'\n";
    int s5 = upload_pdf_to_localhost(f5, "/upload", max_size, 8000, resp);
    std::cout << "Test5: status=" << s5 << " err='" << resp << "'\n";

    // Cleanup
    unlink(f1.c_str());
    unlink(f2.c_str());
    unlink(f3.c_str());
    unlink(f4.c_str());
    unlink(f5.c_str());
    rmdir(base.c_str());
    return 0;
}