#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

std::string write_secret(const std::string& secret) {
    if (secret.empty()) {
        // Allow empty but still create file; could enforce non-empty if desired
    }

    char tmpl[] = "/tmp/secret_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        throw std::runtime_error("mkstemp failed");
    }

    // Ensure owner read/write during write
    (void)fchmod(fd, 0600);

    const char* buf = secret.c_str();
    size_t len = secret.size();
    size_t off = 0;
    while (off < len) {
        ssize_t n = write(fd, buf + off, len - off);
        if (n < 0) {
            int saved = errno;
            close(fd);
            // Best effort to remove the partially written file
            unlink(tmpl);
            throw std::runtime_error(std::string("write failed: ") + strerror(saved));
        }
        off += static_cast<size_t>(n);
    }

    (void)fsync(fd);
    close(fd);

    // Set to read-only for the owner
    if (fchmod(open(tmpl, O_RDONLY), 0400) == -1) {
        // Try chmod path directly if fchmod above failed
        chmod(tmpl, 0400);
    }

    return std::string(tmpl);
}

int main() {
    std::vector<std::string> tests = {
        "super secret info #1",
        "super secret info #2",
        "super secret info #3",
        "super secret info #4",
        "super secret info #5"
    };

    for (const auto& s : tests) {
        try {
            std::string path = write_secret(s);
            std::cout << path << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
        }
    }
    return 0;
}