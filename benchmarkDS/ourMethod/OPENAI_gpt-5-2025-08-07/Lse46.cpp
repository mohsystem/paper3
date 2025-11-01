#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <regex>
#include <iostream>

static const size_t MAX_SIZE = 5 * 1024 * 1024;

static bool is_valid_name(const std::string &name) {
    static const std::regex pattern("^[A-Za-z0-9._-]{1,100}$");
    if (!std::regex_match(name, pattern)) return false;
    if (name.find("..") != std::string::npos) return false;
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) return false;
    return true;
}

// Securely open and read images/name into out buffer. Returns 0 on success.
// Error codes: -2 invalid, -1 not found, -3 too large or IO error.
int open_image(const std::string &name, std::vector<unsigned char> &out) {
    out.clear();
    if (!is_valid_name(name)) return -2;

    int dirfd = open("images", O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) return -1;

    int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    int fd = openat(dirfd, name.c_str(), flags);
    if (fd < 0) {
        close(dirfd);
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        close(dirfd);
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        close(dirfd);
        return -1;
    }
    if (st.st_size < 0 || static_cast<size_t>(st.st_size) > MAX_SIZE) {
        close(fd);
        close(dirfd);
        return -3;
    }

    size_t size = static_cast<size_t>(st.st_size);
    out.resize(size);
    size_t off = 0;
    while (off < size) {
        ssize_t r = read(fd, out.data() + off, size - off);
        if (r < 0) {
            out.clear();
            close(fd);
            close(dirfd);
            return -3;
        }
        if (r == 0) break;
        off += static_cast<size_t>(r);
    }
    out.resize(off);
    close(fd);
    close(dirfd);
    return 0;
}

// Minimal demo handler emulating /img?name=...
std::string handle_img_request(const std::string &name_param) {
    std::vector<unsigned char> data;
    int rc = open_image(name_param, data);
    if (rc == -2) return "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
    if (rc == -1) return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    if (rc != 0) return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nContent-Length: " + std::to_string(data.size()) + "\r\n\r\n";
    std::string body(reinterpret_cast<const char*>(data.data()), data.size());
    return header + body;
}

int main() {
    const char* tests[5] = {"test.png", "a.txt", "../etc/passwd", "nonexistent.xxx", "valid-name.jpg"};
    for (const char* t : tests) {
        std::vector<unsigned char> buf;
        int rc = open_image(std::string(t), buf);
        if (rc == 0) {
            std::cout << "Test [" << t << "]: OK, size=" << buf.size() << "\n";
        } else if (rc == -1) {
            std::cout << "Test [" << t << "]: NotFound\n";
        } else if (rc == -2) {
            std::cout << "Test [" << t << "]: Rejected\n";
        } else {
            std::cout << "Test [" << t << "]: Error\n";
        }
    }
    // Example of building an HTTP response:
    // std::string resp = handle_img_request("test.png");
    // std::cout << resp.substr(0, resp.find("\r\n\r\n")+4) << "(body omitted)\n";
    return 0;
}