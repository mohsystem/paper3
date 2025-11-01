#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <ctime>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

static bool send_all(int sock, const char* data, size_t len) {
    size_t total = 0;
    while (total < len) {
        ssize_t sent = send(sock, data + total, len - total, 0);
        if (sent <= 0) return false;
        total += static_cast<size_t>(sent);
    }
    return true;
}

static int parse_status_code(const std::string& response) {
    size_t pos = response.find("HTTP/");
    if (pos == std::string::npos) return -1;
    size_t sp = response.find(' ', pos);
    if (sp == std::string::npos) return -1;
    size_t sp2 = response.find(' ', sp + 1);
    if (sp2 == std::string::npos) sp2 = response.find('\r', sp + 1);
    if (sp2 == std::string::npos) return -1;
    std::string codeStr = response.substr(sp + 1, sp2 - (sp + 1));
    return std::atoi(codeStr.c_str());
}

int uploadPdf(const std::string& host, const std::string& port, const std::string& path, const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "File not found: " << filePath << std::endl;
        return -1;
    }
    file.seekg(0, std::ios::end);
    std::streamoff fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    if (fileSize < 0) {
        std::cerr << "Failed to get file size: " << filePath << std::endl;
        return -1;
    }

    std::string boundary = "----LSE65Boundary" + std::to_string(std::time(nullptr));
    const std::string CRLF = "\r\n";

    std::string filename;
    {
        size_t pos = filePath.find_last_of("/\\");
        filename = (pos == std::string::npos) ? filePath : filePath.substr(pos + 1);
    }

    std::ostringstream pre;
    pre << "--" << boundary << CRLF;
    pre << "Content-Disposition: form-data; name=\"destination\"" << CRLF << CRLF;
    pre << "uploads" << CRLF;

    pre << "--" << boundary << CRLF;
    pre << "Content-Disposition: form-data; name=\"file\"; filename=\"" << filename << "\"" << CRLF;
    pre << "Content-Type: application/pdf" << CRLF << CRLF;

    std::string preBody = pre.str();
    std::string postBody = CRLF + std::string("--") + boundary + std::string("--") + CRLF;

    long long contentLength = static_cast<long long>(preBody.size()) + static_cast<long long>(fileSize) + static_cast<long long>(postBody.size());

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;
    int gai = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
    if (gai != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(gai) << std::endl;
        return -1;
    }

    int sock = -1;
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) continue;
        if (connect(sock, p->ai_addr, p->ai_addrlen) == 0) break;
        close(sock);
        sock = -1;
    }
    freeaddrinfo(res);

    if (sock == -1) {
        std::cerr << "Failed to connect to " << host << ":" << port << std::endl;
        return -1;
    }

    std::ostringstream hdr;
    hdr << "POST " << path << " HTTP/1.1" << CRLF;
    hdr << "Host: " << host << ":" << port << CRLF;
    hdr << "Content-Type: multipart/form-data; boundary=" << boundary << CRLF;
    hdr << "Content-Length: " << contentLength << CRLF;
    hdr << "Connection: close" << CRLF << CRLF;

    std::string header = hdr.str();

    if (!send_all(sock, header.c_str(), header.size())) {
        std::cerr << "Failed sending header" << std::endl;
        close(sock);
        return -1;
    }
    if (!send_all(sock, preBody.c_str(), preBody.size())) {
        std::cerr << "Failed sending pre-body" << std::endl;
        close(sock);
        return -1;
    }

    // Send file content
    {
        const size_t BUF = 8192;
        std::vector<char> buffer(BUF);
        while (file) {
            file.read(buffer.data(), buffer.size());
            std::streamsize got = file.gcount();
            if (got > 0) {
                if (!send_all(sock, buffer.data(), static_cast<size_t>(got))) {
                    std::cerr << "Failed sending file data" << std::endl;
                    close(sock);
                    return -1;
                }
            }
        }
    }

    if (!send_all(sock, postBody.c_str(), postBody.size())) {
        std::cerr << "Failed sending post-body" << std::endl;
        close(sock);
        return -1;
    }

    // Read response
    std::string response;
    char rbuf[4096];
    ssize_t r;
    while ((r = recv(sock, rbuf, sizeof(rbuf), 0)) > 0) {
        response.append(rbuf, r);
    }
    close(sock);

    int status = parse_status_code(response);
    return status;
}

int main() {
    std::string host = "127.0.0.1";
    std::string port = "8080";
    std::string path = "/upload";

    std::vector<std::string> files = {
        "./sample1.pdf",
        "./sample2.pdf",
        "./docs/test.pdf",
        "./data/report.pdf",
        "./dummy.pdf"
    };

    for (const auto& f : files) {
        int status = uploadPdf(host, port, path, f);
        std::cout << "Upload: " << f << " -> HTTP Status: " << status << std::endl;
    }
    return 0;
}