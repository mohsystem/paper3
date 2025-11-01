#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>

static std::string mode_to_perm_string(mode_t mode) {
    std::string s;
    s.push_back((mode & S_IRUSR) ? 'r' : '-');
    s.push_back((mode & S_IWUSR) ? 'w' : '-');
    s.push_back((mode & S_IXUSR) ? 'x' : '-');
    s.push_back((mode & S_IRGRP) ? 'r' : '-');
    s.push_back((mode & S_IWGRP) ? 'w' : '-');
    s.push_back((mode & S_IXGRP) ? 'x' : '-');
    s.push_back((mode & S_IROTH) ? 'r' : '-');
    s.push_back((mode & S_IWOTH) ? 'w' : '-');
    s.push_back((mode & S_IXOTH) ? 'x' : '-');
    std::ostringstream oss;
    oss << std::oct;
    oss.width(4);
    oss.fill('0');
    oss << (mode & 0777);
    oss << " " << s;
    return oss.str();
}

std::string secure_write_restrict_print(const std::string& path, const std::string& content) {
    int fd = -1;
    const mode_t create_mode = 0600;
    fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, create_mode);
    if (fd == -1) {
        return std::string("ERROR: open failed: ") + std::strerror(errno);
    }

    // Write loop to handle partial writes
    const char* buf = content.c_str();
    size_t total = content.size();
    ssize_t n;
    size_t written = 0;
    while (written < total) {
        n = ::write(fd, buf + written, total - written);
        if (n < 0) {
            int err = errno;
            ::close(fd);
            return std::string("ERROR: write failed: ") + std::strerror(err);
        }
        written += static_cast<size_t>(n);
    }

    // Enforce restrictive permissions regardless of umask
    if (fchmod(fd, 0600) == -1) {
        int err = errno;
        ::close(fd);
        return std::string("ERROR: fchmod failed: ") + std::strerror(err);
    }

    if (::close(fd) == -1) {
        return std::string("ERROR: close failed: ") + std::strerror(errno);
    }

    struct stat st;
    if (stat(path.c_str(), &st) == -1) {
        return std::string("ERROR: stat failed: ") + std::strerror(errno);
    }
    return mode_to_perm_string(st.st_mode);
}

int main() {
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    std::string path = "secret.txt";
    std::cout << secure_write_restrict_print(path, content) << std::endl;
    std::cout << secure_write_restrict_print(path, content) << std::endl;
    std::cout << secure_write_restrict_print(path, content) << std::endl;
    std::cout << secure_write_restrict_print(path, content) << std::endl;
    std::cout << secure_write_restrict_print(path, content) << std::endl;
    return 0;
}