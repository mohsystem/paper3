#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t TAR_BLOCK = 512;
static const size_t MAX_ENTRY_SIZE = 100 * 1024 * 1024; // 100MB per entry

struct TarHeader {
    unsigned char buf[512];
};

static bool isAllZero(const unsigned char* b, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if (b[i] != 0) return false;
    }
    return true;
}

static long long parseOctal(const unsigned char* p, size_t off, size_t len) {
    long long val = 0;
    size_t i = off;
    while (i < off + len && (p[i] == 0 || p[i] == ' ')) i++;
    for (; i < off + len; ++i) {
        unsigned char c = p[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') return -1;
        val = (val << 3) + (c - '0');
        if (val < 0) return -1;
    }
    return val;
}

static bool validChecksum(const unsigned char* h) {
    long long stored = parseOctal(h, 148, 8);
    if (stored < 0) return false;
    long long sum = 0;
    for (int i = 0; i < 512; ++i) {
        if (i >= 148 && i < 156) sum += 32;
        else sum += (unsigned char)h[i];
    }
    return sum == stored;
}

static std::string extractString(const unsigned char* b, size_t off, size_t len) {
    size_t end = off + len;
    size_t i = off;
    while (i < end && b[i] != 0) i++;
    return std::string(reinterpret_cast<const char*>(b + off), i - off);
}

static bool isSafeRelPath(const std::string& p) {
    if (p.empty()) return false;
    if (p[0] == '/') return false;
    if (p.find("..") != std::string::npos) return false;
    if (p.find('\\') != std::string::npos) return false;
    if (p.size() >= 2 && p[1] == ':') return false;
    return true;
}

static bool splitPath(const std::string& p, std::vector<std::string>& outParts) {
    outParts.clear();
    size_t start = 0;
    while (start < p.size()) {
        size_t pos = p.find('/', start);
        std::string part = p.substr(start, pos == std::string::npos ? std::string::npos : pos - start);
        if (!part.empty() && part != ".") outParts.push_back(part);
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    return true;
}

static bool mkdirsAt(int basefd, const std::string& rel) {
    if (rel.empty()) return true;
    std::vector<std::string> parts;
    if (!splitPath(rel, parts)) return false;
    int cur = dup(basefd);
    if (cur < 0) return false;
    for (const auto& part : parts) {
        int dfd = openat(cur, part.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (dfd >= 0) {
            close(cur);
            cur = dfd;
            continue;
        }
        if (errno != ENOENT) {
            close(cur);
            return false;
        }
        if (mkdirat(cur, part.c_str(), 0700) < 0) {
            if (errno != EEXIST) {
                close(cur);
                return false;
            }
        }
        dfd = openat(cur, part.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (dfd < 0) {
            close(cur);
            return false;
        }
        close(cur);
        cur = dfd;
    }
    close(cur);
    return true;
}

static bool skipFully(int fd, size_t n) {
    char buf[8192];
    size_t remaining = n;
    while (remaining > 0) {
        size_t toRead = remaining < sizeof(buf) ? remaining : sizeof(buf);
        ssize_t r = read(fd, buf, toRead);
        if (r < 0) return false;
        if (r == 0) return false;
        remaining -= (size_t)r;
    }
    return true;
}

static bool readFully(int fd, void* buf, size_t n) {
    size_t off = 0;
    while (off < n) {
        ssize_t r = read(fd, (char*)buf + off, n - off);
        if (r < 0) return false;
        if (r == 0) return false;
        off += (size_t)r;
    }
    return true;
}

static bool copyLimited(int fromfd, int tofd, size_t n) {
    char buf[32768];
    size_t remaining = n;
    while (remaining > 0) {
        size_t toRead = remaining < sizeof(buf) ? remaining : sizeof(buf);
        ssize_t r = read(fromfd, buf, toRead);
        if (r <= 0) return false;
        size_t off = 0;
        while (off < (size_t)r) {
            ssize_t w = write(tofd, buf + off, (size_t)r - off);
            if (w <= 0) return false;
            off += (size_t)w;
        }
        remaining -= (size_t)r;
    }
    return true;
}

static bool extract_tar(const char* archive_path, const char* dest_dir) {
    if (!archive_path || !dest_dir) {
        std::cerr << "Invalid input\n";
        return false;
    }
    if (strlen(archive_path) > 4096 || strlen(dest_dir) > 4096) {
        std::cerr << "Path too long\n";
        return false;
    }
    // Prepare destination directory
    if (mkdir(dest_dir, 0700) < 0) {
        if (errno != EEXIST) {
            std::cerr << "Failed to prepare destination\n";
            return false;
        }
    }
    int destfd = open(dest_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (destfd < 0) {
        std::cerr << "Failed to open destination\n";
        return false;
    }

    int afd = open(archive_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (afd < 0) {
        std::cerr << "Failed to open archive\n";
        close(destfd);
        return false;
    }

    bool ok = true;
    unsigned char header[512];
    bool sawZero = false;

    while (true) {
        if (!readFully(afd, header, TAR_BLOCK)) {
            ok = false;
            break;
        }
        if (isAllZero(header, 512)) {
            if (sawZero) {
                ok = true;
                break;
            } else {
                sawZero = true;
                continue;
            }
        } else {
            sawZero = false;
        }

        if (!validChecksum(header)) {
            std::cerr << "Invalid header checksum\n";
            ok = false;
            break;
        }

        std::string name = extractString(header, 0, 100);
        std::string prefix = extractString(header, 345, 155);
        std::string rel = prefix.empty() ? name : (prefix + "/" + name);
        char typeflag = (char)header[156];
        long long sizeLL = parseOctal(header, 124, 12);
        if (sizeLL < 0 || (size_t)sizeLL > MAX_ENTRY_SIZE) {
            std::cerr << "Invalid size\n";
            ok = false;
            break;
        }
        size_t size = (size_t)sizeLL;

        if (!isSafeRelPath(rel)) {
            std::cerr << "Unsafe path\n";
            ok = false;
            break;
        }

        // Determine parent and filename
        std::string parent;
        std::string base;
        size_t pos = rel.rfind('/');
        if (pos == std::string::npos) {
            parent = "";
            base = rel;
        } else {
            parent = rel.substr(0, pos);
            base = rel.substr(pos + 1);
        }

        if (typeflag == '5') {
            if (!mkdirsAt(destfd, rel)) {
                std::cerr << "Failed to create directory\n";
                ok = false;
                break;
            }
            // directory payload (if any) should be skipped
            size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
            if (!skipFully(afd, size + pad)) { ok = false; break; }
        } else if (typeflag == '0' || typeflag == 0) {
            if (!parent.empty()) {
                if (!mkdirsAt(destfd, parent)) {
                    std::cerr << "Failed to create parent dirs\n";
                    ok = false;
                    break;
                }
            }
            int pfd = destfd;
            if (!parent.empty()) {
                pfd = openat(destfd, parent.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
                if (pfd < 0) {
                    std::cerr << "Open parent failed\n";
                    ok = false;
                    break;
                }
            }
            int ofd = openat(pfd, base.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
            if (!parent.empty()) close(pfd);
            if (ofd < 0) {
                std::cerr << "Create output failed\n";
                ok = false;
                break;
            }
            if (!copyLimited(afd, ofd, size)) {
                std::cerr << "Truncated file data\n";
                close(ofd);
                ok = false;
                break;
            }
            fsync(ofd);
            close(ofd);
            size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
            if (pad > 0) {
                if (!skipFully(afd, pad)) { ok = false; break; }
            }
        } else {
            // Unsupported: skip payload safely
            if (size > 0) {
                if (!skipFully(afd, size)) { ok = false; break; }
                size_t pad = (TAR_BLOCK - (size % TAR_BLOCK)) % TAR_BLOCK;
                if (pad > 0) {
                    if (!skipFully(afd, pad)) { ok = false; break; }
                }
            }
        }
    }

    fsync(destfd);
    close(afd);
    close(destfd);
    return ok;
}

int main() {
    std::cout << "Test1: " << (extract_tar("archive.zip", "/tmp/unpack") ? "true" : "false") << "\n";
    std::cout << "Test2: " << (extract_tar("nonexistent.tar", "/tmp/unpack") ? "true" : "false") << "\n";
    std::cout << "Test3: " << (extract_tar("", "/tmp/unpack") ? "true" : "false") << "\n";
    std::cout << "Test4: " << (extract_tar("archive.zip", "/tmp/unpack/sub/..") ? "true" : "false") << "\n";
    std::cout << "Test5: " << (extract_tar("archive.zip", "/tmp/unpack") ? "true" : "false") << "\n";
    return 0;
}