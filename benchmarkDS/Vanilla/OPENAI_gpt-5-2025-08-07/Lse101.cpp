#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>

bool writeImportantConfig(const std::string& path, const std::string& content) {
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return false;

    bool ok = true;
    const char* data = content.c_str();
    ssize_t remaining = static_cast<ssize_t>(content.size());
    ssize_t offset = 0;
    while (remaining > 0) {
        ssize_t n = ::write(fd, data + offset, remaining);
        if (n <= 0) { ok = false; break; }
        offset += n;
        remaining -= n;
    }

    if (ok) {
        if (::fchmod(fd, 0644) == -1) ok = false;
    }

    ::close(fd);
    return ok;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        bool res = writeImportantConfig("important_config", "important_config");
        std::cout << "Test " << i << ": " << std::boolalpha << res << std::endl;
    }
    return 0;
}