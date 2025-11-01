#include <iostream>
#include <string>
#include <vector>
#include <cerrno>

#if defined(_WIN32)
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#else
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <unistd.h>
#endif

static bool is_simple_filename(const std::string& name) {
    if (name.empty()) return false;
#if defined(_WIN32)
    for (char c : name) {
        if (c == '\\' || c == '/' || c == ':' ) return false;
    }
#else
    if (name.find('/') != std::string::npos) return false;
#endif
    if (name == "." || name == "..") return false;
    return true;
}

bool create_secret_file(const std::string& filename, const std::string& content) {
    if (!is_simple_filename(filename)) return false;

#if defined(_WIN32)
    // Use _sopen_s to create new file exclusively
    int fd = -1;
    errno_t err = _sopen_s(&fd, filename.c_str(), _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY, _SH_DENYRW, _S_IREAD | _S_IWRITE);
    if (err != 0 || fd < 0) {
        return false;
    }
    size_t total = 0;
    while (total < content.size()) {
        int written = _write(fd, content.data() + total, static_cast<unsigned int>(content.size() - total));
        if (written <= 0) {
            _close(fd);
            return false;
        }
        total += static_cast<size_t>(written);
    }
    _close(fd);
    // Restrict permissions: best-effort on Windows CRT
    _chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
    return true;
#else
    int flags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    mode_t mode = S_IRUSR | S_IWUSR; // 0600
    int fd = ::open(filename.c_str(), flags, mode);
    if (fd < 0) {
        return false;
    }
    const char* data = content.data();
    size_t len = content.size();
    size_t total = 0;
    while (total < len) {
        ssize_t w = ::write(fd, data + total, len - total);
        if (w <= 0) {
            ::close(fd);
            return false;
        }
        total += static_cast<size_t>(w);
    }
    // Ensure restrictive permissions
    ::fchmod(fd, mode);
    ::close(fd);
    ::chmod(filename.c_str(), mode);
    return true;
#endif
}

int main() {
    const std::string secret = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    std::cout << (create_secret_file("secret.txt", secret) ? "1" : "0") << std::endl;
    std::cout << (create_secret_file("secret1.txt", secret) ? "1" : "0") << std::endl;
    std::cout << (create_secret_file("secret2.txt", secret) ? "1" : "0") << std::endl;
    std::cout << (create_secret_file("secret3.txt", secret) ? "1" : "0") << std::endl;
    std::cout << (create_secret_file("secret4.txt", secret) ? "1" : "0") << std::endl;
    return 0;
}