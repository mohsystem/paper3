#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t MAX_SIZE = 8 * 1024 * 1024; // 8 MiB

struct Result {
    uint64_t bytesRead;
    uint32_t checksum;
    bool success;
    std::string message;
};

static bool is_safe_rel_path(const std::string& rel) {
    if (rel.empty()) return false;
    if (rel[0] == '/' || rel[0] == '\\') return false;
    if (rel.find('\0') != std::string::npos) return false;
    size_t start = 0;
    while (start <= rel.size()) {
        size_t slash = rel.find('/', start);
        std::string part = rel.substr(start, (slash == std::string::npos) ? std::string::npos : slash - start);
        if (part.empty() || part == "." || part == ".." || part.find('\\') != std::string::npos) return false;
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    return true;
}

static bool ensure_dirs(int dirfd, const std::string& relpath) {
    size_t pos = 0;
    size_t last = relpath.rfind('/');
    if (last == std::string::npos) return true; // no directories
    std::string cur;
    while (pos < last) {
        size_t next = relpath.find('/', pos);
        if (next == std::string::npos || next > last) next = last;
        std::string part = relpath.substr(pos, next - pos);
        if (!cur.empty()) cur += "/";
        cur += part;
        if (mkdirat(dirfd, cur.c_str(), 0700) != 0) {
            if (errno == EEXIST) {
                struct stat st;
                if (fstatat(dirfd, cur.c_str(), &st, AT_SYMLINK_NOFOLLOW) != 0) return false;
                if (!S_ISDIR(st.st_mode)) return false;
            } else {
                return false;
            }
        }
        pos = next + 1;
    }
    return true;
}

static bool gen_unique_tmp(int dirfd, std::string& name_out) {
    for (int attempt = 0; attempt < 100; ++attempt) {
        char buf[64];
        unsigned int r = (unsigned int) getpid() ^ (unsigned int) time(nullptr) ^ (unsigned int) attempt;
        snprintf(buf, sizeof(buf), ".tmp_%u_%d", r, attempt);
        int flags = O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW;
        int fd = openat(dirfd, buf, flags, 0600);
        if (fd >= 0) {
            close(fd);
            name_out = buf;
            return true;
        } else {
            if (errno == EEXIST) continue;
        }
    }
    return false;
}

Result process_file(const std::string& baseDir, const std::string& inputRel, const std::string& outputRel) {
    if (!is_safe_rel_path(inputRel) || !is_safe_rel_path(outputRel)) {
        return {0, 0, false, "Invalid relative path"};
    }

    int bfd = open(baseDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (bfd < 0) {
        return {0, 0, false, "Base directory open failed"};
    }

    // Open input file securely
    int ifd = openat(bfd, inputRel.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (ifd < 0) {
        close(bfd);
        return {0, 0, false, "Input open failed"};
    }

    struct stat st;
    if (fstat(ifd, &st) != 0) {
        close(ifd);
        close(bfd);
        return {0, 0, false, "fstat failed"};
    }
    if (!S_ISREG(st.st_mode)) {
        close(ifd);
        close(bfd);
        return {0, 0, false, "Not a regular file"};
    }
    if ((size_t)st.st_size > MAX_SIZE) {
        close(ifd);
        close(bfd);
        return {0, 0, false, "File too large"};
    }

    uint64_t bytesRead = 0;
    uint32_t checksum = 0;
    std::vector<unsigned char> buf(8192);
    while (true) {
        ssize_t n = read(ifd, buf.data(), buf.size());
        if (n < 0) {
            close(ifd);
            close(bfd);
            return {0, 0, false, "Read failed"};
        }
        if (n == 0) break;
        bytesRead += (uint64_t)n;
        for (ssize_t i = 0; i < n; ++i) {
            checksum = (checksum + buf[(size_t)i]) & 0xFFFFFFFFu;
        }
    }
    close(ifd);

    if (!ensure_dirs(bfd, outputRel)) {
        close(bfd);
        return {0, 0, false, "Directory creation failed"};
    }

    // Create temp file in same directory
    std::string tmpName;
    if (!gen_unique_tmp(bfd, tmpName)) {
        close(bfd);
        return {0, 0, false, "Temp create failed"};
    }

    int tfd = openat(bfd, tmpName.c_str(), O_WRONLY | O_CLOEXEC | O_NOFOLLOW);
    if (tfd < 0) {
        unlinkat(bfd, tmpName.c_str(), 0);
        close(bfd);
        return {0, 0, false, "Temp open failed"};
    }

    char outbuf[128];
    int len = snprintf(outbuf, sizeof(outbuf), "%llu,%u\n",
                       (unsigned long long)bytesRead, (unsigned int)checksum);
    ssize_t off = 0;
    while (off < len) {
        ssize_t w = write(tfd, outbuf + off, (size_t)(len - off));
        if (w <= 0) {
            close(tfd);
            unlinkat(bfd, tmpName.c_str(), 0);
            close(bfd);
            return {0, 0, false, "Write failed"};
        }
        off += w;
    }
    if (fsync(tfd) != 0) {
        close(tfd);
        unlinkat(bfd, tmpName.c_str(), 0);
        close(bfd);
        return {0, 0, false, "fsync failed"};
    }
    close(tfd);

    // Move into place
    if (renameat(bfd, tmpName.c_str(), bfd, outputRel.c_str()) != 0) {
        // try replace
        unlinkat(bfd, outputRel.c_str(), 0);
        if (renameat(bfd, tmpName.c_str(), bfd, outputRel.c_str()) != 0) {
            unlinkat(bfd, tmpName.c_str(), 0);
            close(bfd);
            return {0, 0, false, "Rename failed"};
        }
    }

    // fsync directory
    fsync(bfd);
    close(bfd);

    return {bytesRead, checksum, true, "OK"};
}

static bool write_file(int dirfd, const std::string& rel, const std::vector<unsigned char>& data) {
    if (!ensure_dirs(dirfd, rel)) return false;
    int fd = openat(dirfd, rel.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return false;
    ssize_t off = 0;
    while ((size_t)off < data.size()) {
        ssize_t w = write(fd, data.data() + off, data.size() - (size_t)off);
        if (w <= 0) { close(fd); return false; }
        off += w;
    }
    fsync(fd);
    close(fd);
    return true;
}

int main() {
    char templ[] = "/tmp/secure_io_base_XXXXXX";
    char* basedir = mkdtemp(templ);
    if (!basedir) {
        std::cerr << "mkdtemp failed\n";
        return 1;
    }
    std::cout << "Base dir: " << basedir << "\n";
    int bfd = open(basedir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (bfd < 0) {
        std::cerr << "open base failed\n";
        return 1;
    }

    // Prepare test inputs
    write_file(bfd, "empty.txt", {});
    std::vector<unsigned char> hello({'H','e','l','l','o',',',' ','W','o','r','l','d','!'});
    write_file(bfd, "hello.txt", hello);
    std::vector<unsigned char> zeros(1024, 0);
    write_file(bfd, "zeros.bin", zeros);
    std::vector<unsigned char> repeated;
    for (int i = 0; i < 1000; ++i) { repeated.push_back('a'); repeated.push_back('b'); repeated.push_back('c'); }
    write_file(bfd, "repeated.txt", repeated);
    close(bfd);

    Result r1 = process_file(basedir, "empty.txt", "out/out1.txt");
    std::cout << "Test1: bytes=" << r1.bytesRead << " checksum=" << r1.checksum << " success=" << r1.success << " msg=" << r1.message << "\n";

    Result r2 = process_file(basedir, "hello.txt", "out/out2.txt");
    std::cout << "Test2: bytes=" << r2.bytesRead << " checksum=" << r2.checksum << " success=" << r2.success << " msg=" << r2.message << "\n";

    Result r3 = process_file(basedir, "zeros.bin", "out/out3.txt");
    std::cout << "Test3: bytes=" << r3.bytesRead << " checksum=" << r3.checksum << " success=" << r3.success << " msg=" << r3.message << "\n";

    Result r4 = process_file(basedir, "repeated.txt", "out/out4.txt");
    std::cout << "Test4: bytes=" << r4.bytesRead << " checksum=" << r4.checksum << " success=" << r4.success << " msg=" << r4.message << "\n";

    Result r5 = process_file(basedir, "../evil.txt", "out/out5.txt");
    std::cout << "Test5: bytes=" << r5.bytesRead << " checksum=" << r5.checksum << " success=" << r5.success << " msg=" << r5.message << "\n";

    return 0;
}