#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

static bool isValidUsername(const std::string &s) {
    if (s.empty() || s.size() > 64) return false;
    for (unsigned char ch : s) {
        if (std::isalnum(ch)) continue;
        if (ch == '_' || ch == '.' || ch == '-') continue;
        return false;
    }
    return true;
}

static bool isSafePath(const std::string &p) {
    if (p.empty() || p.size() > 4096) return false;
    if (p.find("..") != std::string::npos) return false;
    return true;
}

bool usernameExists(const std::string &dbPath, const std::string &username) {
    if (!isValidUsername(username)) return false;
    if (!isSafePath(dbPath)) return false;

    // Check it's a regular file and not a symlink
    struct stat st{};
    if (lstat(dbPath.c_str(), &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    if (S_ISLNK(st.st_mode)) return false;

    std::ifstream in(dbPath);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        if (line.size() > 256) {
            // Skip unusually long records
            continue;
        }
        // Remove trailing '\r' if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (isValidUsername(line) && line == username) {
            return true;
        }
    }
    return false;
}

static std::string createSampleDb() {
    char tmpl[] = "/tmp/usersXXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) {
        std::cerr << "Setup error.\n";
        std::exit(1);
    }
    fchmod(fd, 0600);
    const char *data =
        "alice\n"
        "Bob_123\n"
        "charlie\n"
        "david.s\n"
        "eve-9\n";
    ssize_t to_write = (ssize_t)std::strlen(data);
    ssize_t written = write(fd, data, (size_t)to_write);
    if (written != to_write) {
        close(fd);
        unlink(tmpl);
        std::cerr << "Setup write error.\n";
        std::exit(1);
    }
    close(fd);
    return std::string(tmpl);
}

int main() {
    std::string dbPath = createSampleDb();

    std::vector<std::string> tests = {
        "alice",    // true
        "bob_123",  // false (case-sensitive)
        "charlie",  // true
        "mallory",  // false
        "eve-9"     // true
    };

    for (const auto &t : tests) {
        bool result = usernameExists(dbPath, t);
        std::cout << t << " -> " << (result ? "true" : "false") << "\n";
    }

    unlink(dbPath.c_str());
    return 0;
}