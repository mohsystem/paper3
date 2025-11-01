#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>

static const size_t MAX_PATH_LEN = 4096;
static const char* CONTENT = "important_config";

static bool split_components(const std::string& rel, std::vector<std::string>& out) {
    if (rel.empty()) return false;
    if (rel[0] == '/') return false;
    std::string cur;
    for (size_t i = 0; i <= rel.size(); ++i) {
        if (i == rel.size() || rel[i] == '/') {
            if (cur.empty() || cur == "." || cur == "..") return false;
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(rel[i]);
        }
    }
    return !out.empty();
}

static bool safe_write_important_config(const std::string& base_dir, const std::string& rel_path) {
    if (base_dir.empty() || rel_path.empty()) return false;
    if (rel_path.size() > MAX_PATH_LEN) return false;
    if (rel_path.find('\0') != std::string::npos || base_dir.find('\0') != std::string::npos) return false;

    std::vector<std::string> parts;
    if (!split_components(rel_path, parts)) return false;

    int basefd = open(base_dir.c_str(), O_RDONLY | O_CLOEXEC);
    if (basefd < 0) return false;

    struct stat st;
    if (fstat(basefd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(basefd);
        return false;
    }

    int dirfd = basefd;
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        int nfd = openat(dirfd, parts[i].c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
        if (nfd < 0) {
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return false;
        }
        struct stat st2;
        if (fstat(nfd, &st2) != 0 || !S_ISDIR(st2.st_mode)) {
            close(nfd);
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return false;
        }
        if (dirfd != basefd) close(dirfd);
        dirfd = nfd;
    }

    const std::string& finalName = parts.back();
    if (finalName.empty() || finalName.size() > 255) {
        if (dirfd != basefd) close(dirfd);
        close(basefd);
        return false;
    }

    // Best effort: if destination exists and is symlink, reject (lstat via openat + O_NOFOLLOW)
    int dst_check_fd = openat(dirfd, finalName.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (dst_check_fd >= 0) {
        struct stat st3;
        bool bad = false;
        if (fstat(dst_check_fd, &st3) == 0) {
            // If it's a symlink, O_NOFOLLOW should have failed with ELOOP; but check regularity.
            // Proceed; if not regular, we can still replace atomically. We'll just proceed.
        }
        close(dst_check_fd);
        (void)bad;
    } else {
        if (errno == ELOOP) {
            if (dirfd != basefd) close(dirfd);
            close(basefd);
            return false;
        }
    }

    // Create temp file
    unsigned char rnd[8];
    {
        int ur = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
        if (ur >= 0) {
            ssize_t got = read(ur, rnd, sizeof(rnd));
            close(ur);
            if (got != (ssize_t)sizeof(rnd)) {
                // Fallback to pid/time
                for (size_t i = 0; i < sizeof(rnd); ++i) rnd[i] = (unsigned char)((getpid() + i * 31 + time(nullptr)) & 0xFF);
            }
        } else {
            for (size_t i = 0; i < sizeof(rnd); ++i) rnd[i] = (unsigned char)((getpid() + i * 31 + time(nullptr)) & 0xFF);
        }
    }
    std::ostringstream oss;
    oss << ".tmp-";
    for (unsigned char b : rnd) {
        const char* hex = "0123456789abcdef";
        oss << hex[(b >> 4) & 0xF] << hex[b & 0xF];
    }
    oss << "-" << getpid();
    std::string tmpname = oss.str();

    int tfd = openat(dirfd, tmpname.c_str(),
                     O_RDWR | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (tfd < 0) {
        if (dirfd != basefd) close(dirfd);
        close(basefd);
        return false;
    }

    bool ok = false;
    do {
        struct stat stt;
        if (fstat(tfd, &stt) != 0 || !S_ISREG(stt.st_mode)) break;

        size_t len = strlen(CONTENT);
        const char* p = CONTENT;
        while (len > 0) {
            ssize_t w = write(tfd, p, len);
            if (w < 0) { ok = false; break; }
            p += (size_t)w;
            len -= (size_t)w;
        }
        if (len != 0) break;

        if (fsync(tfd) != 0) break;
        if (close(tfd) != 0) { tfd = -1; break; }
        tfd = -1;

        if (renameat(dirfd, tmpname.c_str(), dirfd, finalName.c_str()) != 0) break;

        if (fsync(dirfd) != 0) break;

        ok = true;
    } while (0);

    if (!ok) {
        if (tfd >= 0) close(tfd);
        unlinkat(dirfd, tmpname.c_str(), 0);
    }

    if (dirfd != basefd) close(dirfd);
    close(basefd);
    return ok;
}

int main() {
    // Prepare base directory
    char tmpl[] = "/tmp/safe-base-XXXXXX";
    char* base = mkdtemp(tmpl);
    if (!base) {
        std::cout << "Setup failed\n";
        return 0;
    }
    // Create subdir
    std::string baseStr(base);
    std::string subdir = baseStr + "/subdir";
    mkdir(subdir.c_str(), 0700);

    std::vector<std::string> tests = {
        "config.txt",            // OK
        "subdir/settings.conf",  // OK
        "../escape.txt",         // FAIL
        "/abs/path.txt",         // FAIL
        "nested/../final.conf"   // FAIL
    };

    for (const auto& rel : tests) {
        bool res = safe_write_important_config(baseStr, rel);
        std::cout << rel << " -> " << (res ? "OK" : "FAIL") << "\n";
    }
    return 0;
}