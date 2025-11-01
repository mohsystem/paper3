#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#if defined(_WIN32) || defined(_WIN64)
// POSIX permissions not available; we'll compile but note that chmod 0400 may not behave as desired.
#include <io.h>
#include <fcntl.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

static std::string writeSecretToReadOnlyFile(const std::string& secret, const std::string& preferredFilename = std::string()) {
    std::string path;
    int fd = -1;

#if defined(_WIN32) || defined(_WIN64)
    // On Windows, emulate as much as possible
    if (!preferredFilename.empty()) {
        path = preferredFilename;
        // _sopen_s for exclusivity; set permissions via _chmod later
        int oflag = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
        int pmode = _S_IREAD | _S_IWRITE; // 0600-ish
        if (_sopen_s(&fd, path.c_str(), oflag, _SH_DENYNO, pmode) != 0) {
            throw std::runtime_error(std::string("Failed to create file: ") + strerror(errno));
        }
    } else {
        char tmpPath[MAX_PATH];
        char tmpDir[MAX_PATH];
        if (GetTempPathA(MAX_PATH, tmpDir) == 0) {
            throw std::runtime_error("GetTempPath failed");
        }
        if (GetTempFileNameA(tmpDir, "sec", 0, tmpPath) == 0) {
            throw std::runtime_error("GetTempFileName failed");
        }
        path = tmpPath;
        // reopen with exclusive create to ensure fresh permissions
        // Delete the pre-created file and recreate exclusively
        _unlink(path.c_str());
        int oflag = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
        int pmode = _S_IREAD | _S_IWRITE;
        if (_sopen_s(&fd, path.c_str(), oflag, _SH_DENYNO, pmode) != 0) {
            throw std::runtime_error(std::string("Failed to create temp file: ") + strerror(errno));
        }
    }

    // Write
    if (_write(fd, secret.data(), (unsigned int)secret.size()) < 0) {
        int err = errno;
        _close(fd);
        throw std::runtime_error(std::string("Write failed: ") + strerror(err));
    }
    _commit(fd);
    _close(fd);

    // Make read-only
    _chmod(path.c_str(), _S_IREAD);
#else
    // POSIX path
    if (!preferredFilename.empty()) {
        path = preferredFilename;
        // Create parent directories if needed (simple attempt: ignore if missing)
        // For strictness, user should create dirs beforehand.

        fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
        if (fd < 0) {
            throw std::runtime_error(std::string("Failed to create file: ") + std::strerror(errno));
        }
    } else {
        // Create secure temp file
        std::string tmpl = "/tmp/secret_XXXXXX";
        std::vector<char> buf(tmpl.begin(), tmpl.end());
        buf.push_back('\0');
        fd = ::mkstemp(buf.data()); // mode 0600
        if (fd < 0) {
            throw std::runtime_error(std::string("mkstemp failed: ") + std::strerror(errno));
        }
        path = std::string(buf.data());
    }

    // Write secret
    ssize_t total = 0;
    const char* data = secret.data();
    ssize_t remaining = (ssize_t)secret.size();
    while (remaining > 0) {
        ssize_t w = ::write(fd, data + total, (size_t)remaining);
        if (w < 0) {
            int err = errno;
            ::close(fd);
            throw std::runtime_error(std::string("Write failed: ") + std::strerror(err));
        }
        total += w;
        remaining -= w;
    }
    ::fsync(fd);
    ::close(fd);

    // Set read-only for owner
    if (::chmod(path.c_str(), S_IRUSR) != 0) {
        throw std::runtime_error(std::string("chmod failed: ") + std::strerror(errno));
    }
#endif

    return path;
}

int main() {
    try {
        // 5 test cases
        std::string f1 = writeSecretToReadOnlyFile("TopSecret-Alpha");
        std::cout << f1 << std::endl;

        std::string f2 = writeSecretToReadOnlyFile("TopSecret-Bravo", "/tmp/secret_specified_cpp_1.txt");
        std::cout << f2 << std::endl;

        std::string f3 = writeSecretToReadOnlyFile("TopSecret-Charlie");
        std::cout << f3 << std::endl;

        std::string f4 = writeSecretToReadOnlyFile("TopSecret-Delta", "/tmp/secret_specified_cpp_2.txt");
        std::cout << f4 << std::endl;

        std::string f5 = writeSecretToReadOnlyFile("TopSecret-Echo");
        std::cout << f5 << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}