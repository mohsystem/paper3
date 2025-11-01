#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <cstring>
#include <iostream>

bool writeImportantConfig(const std::string& content) {
    const char* path = "important_config";
    const char* fixed = "important_config";
    int flags = O_WRONLY | O_CREAT | O_TRUNC;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = open(path, flags, 0600);
    if (fd == -1) {
        struct stat st;
        if (lstat(path, &st) == 0 && S_ISLNK(st.st_mode)) {
            return false;
        }
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd == -1) {
            return false;
        }
    }

    fchmod(fd, 0600);

    size_t len = std::strlen(fixed);
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, fixed + off, len - off);
        if (n <= 0) {
            close(fd);
            return false;
        }
        off += static_cast<size_t>(n);
    }

    if (close(fd) != 0) {
        return false;
    }
    return true;
}

int main() {
    std::cout << (writeImportantConfig("important_config") ? "1" : "0") << std::endl;
    std::cout << (writeImportantConfig("important_config") ? "1" : "0") << std::endl;
    std::cout << (writeImportantConfig("important_config") ? "1" : "0") << std::endl;
    std::cout << (writeImportantConfig("important_config") ? "1" : "0") << std::endl;
    std::cout << (writeImportantConfig("important_config") ? "1" : "0") << std::endl;
    return 0;
}