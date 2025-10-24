#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cerrno>

#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

static const size_t MAX_PASSWORD_LENGTH = 1024;

bool comparePasswordFromFile(const std::string& filePath, const std::string& userPassword) {
    if (filePath.empty()) return false;

    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        return false;
    }

    std::string stored;
    stored.reserve(64);
    char ch;
    while (in.get(ch)) {
        if (ch == '\n') break;
        if (ch == '\r') {
            char next;
            if (in.peek() == '\n') {
                in.get(next);
            }
            break;
        }
        if (stored.size() >= MAX_PASSWORD_LENGTH) {
            // too long
            stored.clear();
            return false;
        }
        stored.push_back(ch);
    }
    if (!in && !in.eof()) {
        return false;
    }

    // Constant-time compare
    size_t lenA = stored.size();
    size_t lenB = userPassword.size();
    size_t maxLen = (lenA > lenB) ? lenA : lenB;
    unsigned int diff = static_cast<unsigned int>(lenA ^ lenB);
    for (size_t i = 0; i < maxLen; ++i) {
        unsigned char ca = (i < lenA) ? static_cast<unsigned char>(stored[i]) : 0;
        unsigned char cb = (i < lenB) ? static_cast<unsigned char>(userPassword[i]) : 0;
        diff |= static_cast<unsigned int>(ca ^ cb);
    }

    // Attempt to wipe stored in memory
    std::fill(stored.begin(), stored.end(), '\0');

    return diff == 0;
}

#if defined(__unix__) || defined(__APPLE__)
static std::string createSecureTempFile(const std::string& content) {
    std::string tmpl = "/tmp/task89_XXXXXXXX.pwd";
    std::vector<char> path(tmpl.begin(), tmpl.end());
    path.push_back('\0');

    int fd = mkstemp(path.data());
    if (fd == -1) {
        return std::string();
    }
    fchmod(fd, S_IRUSR | S_IWUSR);
    ssize_t toWrite = static_cast<ssize_t>(content.size());
    const char* buf = content.data();
    while (toWrite > 0) {
        ssize_t w = write(fd, buf, static_cast<size_t>(toWrite));
        if (w <= 0) {
            close(fd);
            unlink(path.data());
            return std::string();
        }
        buf += w;
        toWrite -= w;
    }
    close(fd);
    return std::string(path.data());
}
#else
static std::string createSecureTempFile(const std::string& content) {
    // Fallback (not as secure): current directory with random-ish name
    std::string filename = "task89_fallback_" + std::to_string(std::rand()) + ".pwd";
    std::ofstream out(filename, std::ios::binary | std::ios::trunc);
    if (!out) return std::string();
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return filename;
}
#endif

int main() {
    std::string f1 = createSecureTempFile("secret123");
    std::string f2 = createSecureTempFile("secret123");
    std::string f3 = createSecureTempFile("pass with spaces");
    std::string f4 = createSecureTempFile("");
    std::string f5 = createSecureTempFile("short");

    std::cout << (comparePasswordFromFile(f1, "secret123") ? "1" : "0") << std::endl; // 1
    std::cout << (comparePasswordFromFile(f2, "secret124") ? "1" : "0") << std::endl; // 0
    std::cout << (comparePasswordFromFile(f3, "pass with spaces") ? "1" : "0") << std::endl; // 1
    std::cout << (comparePasswordFromFile(f4, "") ? "1" : "0") << std::endl; // 1
    std::cout << (comparePasswordFromFile(f5, "shorter") ? "1" : "0") << std::endl; // 0

#if defined(__unix__) || defined(__APPLE__)
    if (!f1.empty()) unlink(f1.c_str());
    if (!f2.empty()) unlink(f2.c_str());
    if (!f3.empty()) unlink(f3.c_str());
    if (!f4.empty()) unlink(f4.c_str());
    if (!f5.empty()) unlink(f5.c_str());
#else
    if (!f1.empty()) std::remove(f1.c_str());
    if (!f2.empty()) std::remove(f2.c_str());
    if (!f3.empty()) std::remove(f3.c_str());
    if (!f4.empty()) std::remove(f4.c_str());
    if (!f5.empty()) std::remove(f5.c_str());
#endif

    return 0;
}