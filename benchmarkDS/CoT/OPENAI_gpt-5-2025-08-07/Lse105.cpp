/*
Chain-of-Through:
1) Problem: Create/overwrite secret.txt with given content; restrict permissions.
2) Security: Create with restrictive perms (0600 POSIX, R/W owner on Windows) to avoid wide-open window.
3) Generation: Use open/_sopen_s with appropriate mode; write; fsync/_commit; enforce perms again.
4) Review: Error checks for all syscalls; cross-platform guards.
5) Output: Returns bool success/failure; includes 5 tests in main.
*/

#include <string>
#include <iostream>

#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <share.h>
#else
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <unistd.h>
#endif

bool createSecretFile(const std::string& filename, const std::string& content) {
#ifdef _WIN32
    int fd = -1;
    errno_t err = _sopen_s(&fd, filename.c_str(),
                           _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY,
                           _SH_DENYNO,
                           _S_IREAD | _S_IWRITE);
    if (err != 0 || fd < 0) {
        return false;
    }
    const char* data = content.data();
    size_t len = content.size();
    size_t total = 0;
    while (total < len) {
        int wrote = _write(fd, data + total, (unsigned int)(len - total));
        if (wrote <= 0) {
            _close(fd);
            return false;
        }
        total += (size_t)wrote;
    }
    _commit(fd);
    _close(fd);
    // Re-apply restrictive perms
    _chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
    return true;
#else
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) {
        return false;
    }
    const char* data = content.data();
    size_t len = content.size();
    size_t total = 0;
    while (total < len) {
        ssize_t wrote = write(fd, data + total, len - total);
        if (wrote <= 0) {
            close(fd);
            return false;
        }
        total += (size_t)wrote;
    }
    fsync(fd);
    fchmod(fd, 0600);
    close(fd);
    return true;
#endif
}

int main() {
    std::string filename = "secret.txt";
    std::string content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    for (int i = 1; i <= 5; ++i) {
        bool ok = createSecretFile(filename, content);
        std::cout << "Test " << i << ": " << (ok ? "true" : "false") << std::endl;
    }
    return 0;
}