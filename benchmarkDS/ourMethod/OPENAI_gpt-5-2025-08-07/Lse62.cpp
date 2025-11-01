#include <cerrno>
#include <cinttypes>
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

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

static const size_t MAX_FILE_SIZE = 16 * 1024 * 1024; // 16 MiB

struct Result {
    size_t bytesRead;
    uint32_t checksum;
    bool empty;
};

static bool is_safe_relative(const std::string& rel) {
    if (rel.empty() || (!rel.empty() && rel[0] == '/')) return false;
    if (rel.find('\0') != std::string::npos) return false;
    // Normalize-like check
    size_t start = 0;
    while (start < rel.size()) {
        size_t end = rel.find('/', start);
        std::string part = rel.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        if (part == "..") return false;
        start = (end == std::string::npos) ? rel.size() : end + 1;
    }
    return true;
}

static int open_base_dirfd(const std::string& base) {
    int fd = open(base.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    return fd;
}

static int ensure_dir_hierarchy(int base_dirfd, const std::string& dir_rel) {
    if (dir_rel.empty()) {
        // return dup of base
        int dupfd = fcntl(base_dirfd, F_DUPFD_CLOEXEC, 0);
        return dupfd;
    }
    int curfd = fcntl(base_dirfd, F_DUPFD_CLOEXEC, 0);
    if (curfd < 0) return -1;

    size_t start = 0;
    while (start < dir_rel.size()) {
        size_t end = dir_rel.find('/', start);
        std::string comp = dir_rel.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
        start = (end == std::string::npos) ? dir_rel.size() : end + 1;
        if (comp.empty() || comp == "." || comp == "..") { close(curfd); errno = EINVAL; return -1; }

        // mkdir
        if (mkdirat(curfd, comp.c_str(), 0700) < 0) {
            if (errno != EEXIST) { int e = errno; close(curfd); errno = e; return -1; }
        }
        int nextfd = openat(curfd, comp.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
        if (nextfd < 0) { int e = errno; close(curfd); errno = e; return -1; }
        close(curfd);
        curfd = nextfd;
    }
    return curfd;
}

static bool split_dir_file(const std::string& rel, std::string& dir, std::string& leaf) {
    size_t pos = rel.rfind('/');
    if (pos == std::string::npos) {
        dir.clear();
        leaf = rel;
    } else {
        dir = rel.substr(0, pos);
        leaf = rel.substr(pos + 1);
    }
    if (leaf.empty()) return false;
    if (leaf == "." || leaf == "..") return false;
    return true;
}

static int write_atomic_at(int dirfd, const std::string& leaf, const std::string& content) {
    // create temp
    char tmpname[512];
    if (leaf.size() > 400) { errno = ENAMETOOLONG; return -1; }
    snprintf(tmpname, sizeof(tmpname), "%s.tmp-%ld-%ld", leaf.c_str(), (long)getpid(), (long)random());

    int fd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    ssize_t off = 0;
    const char* data = content.c_str();
    ssize_t len = (ssize_t)content.size();
    while (off < len) {
        ssize_t w = write(fd, data + off, (size_t)(len - off));
        if (w < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
        off += w;
    }
    if (fsync(fd) < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    if (close(fd) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    if (renameat(dirfd, tmpname, dirfd, leaf.c_str()) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); errno = e; return -1; }
    if (fsync(dirfd) < 0) { /* best effort */ }
    return 0;
}

Result process_file(const std::string& base_dir, const std::string& input_rel, const std::string& output_rel) {
    Result r{0, 0, true};
    if (!is_safe_relative(input_rel) || !is_safe_relative(output_rel)) {
        throw std::runtime_error("Unsafe relative path");
    }
    // open base dir
    int basedirfd = open_base_dirfd(base_dir);
    if (basedirfd < 0) {
        throw std::runtime_error("Failed to open base directory");
    }

    // open input securely
    int infd = -1;
    struct stat st{};
    do {
        infd = openat(basedirfd, input_rel.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        if (infd < 0) { int e = errno; close(basedirfd); errno = e; throw std::runtime_error("Failed to open input"); }
        if (fstat(infd, &st) < 0) { int e = errno; close(infd); close(basedirfd); errno = e; throw std::runtime_error("fstat failed"); }
        if (!S_ISREG(st.st_mode)) { close(infd); close(basedirfd); throw std::runtime_error("Not a regular file"); }
        if (st.st_size < 0 || (size_t)st.st_size > MAX_FILE_SIZE) { close(infd); close(basedirfd); throw std::runtime_error("File too large"); }
    } while (0);

    // read into buffer
    std::vector<uint8_t> buf;
    buf.resize((size_t)st.st_size);
    size_t total = 0;
    while (total < buf.size()) {
        ssize_t n = read(infd, buf.data() + total, buf.size() - total);
        if (n < 0) { int e = errno; close(infd); close(basedirfd); errno = e; throw std::runtime_error("Read failed"); }
        if (n == 0) break;
        total += (size_t)n;
    }
    close(infd);

    uint32_t checksum = 0;
    for (size_t i = 0; i < total; ++i) {
        checksum = (uint32_t)(checksum + buf[i]);
    }
    r.bytesRead = total;
    r.checksum = checksum;
    r.empty = (checksum == 0);

    // prepare output
    std::string outdir, outleaf;
    if (!split_dir_file(output_rel, outdir, outleaf)) {
        close(basedirfd);
        throw std::runtime_error("Invalid output path");
    }
    int outdirfd = ensure_dir_hierarchy(basedirfd, outdir);
    if (outdirfd < 0) {
        int e = errno;
        close(basedirfd);
        errno = e;
        throw std::runtime_error("Failed to create/open output dirs");
    }

    // write atomic
    char line[128];
    int m = snprintf(line, sizeof(line), "%zu %u\n", r.bytesRead, r.checksum);
    if (m < 0 || (size_t)m >= sizeof(line)) {
        close(outdirfd);
        close(basedirfd);
        throw std::runtime_error("Formatting error");
    }
    if (write_atomic_at(outdirfd, outleaf, std::string(line, (size_t)m)) != 0) {
        int e = errno;
        close(outdirfd);
        close(basedirfd);
        errno = e;
        throw std::runtime_error("Failed to write output");
    }

    close(outdirfd);
    close(basedirfd);
    return r;
}

static void safe_write_input(const std::string& base_dir, const std::string& rel, const std::vector<uint8_t>& data) {
    if (!is_safe_relative(rel)) throw std::runtime_error("Unsafe rel path");
    int basedirfd = open_base_dirfd(base_dir);
    if (basedirfd < 0) throw std::runtime_error("Open base failed");

    std::string d, leaf;
    if (!split_dir_file(rel, d, leaf)) { close(basedirfd); throw std::runtime_error("Bad rel"); }
    int dirfd = ensure_dir_hierarchy(basedirfd, d);
    if (dirfd < 0) { int e = errno; close(basedirfd); errno = e; throw std::runtime_error("Dirs failed"); }

    char tmpname[512];
    if (leaf.size() > 400) { close(dirfd); close(basedirfd); throw std::runtime_error("Name too long"); }
    snprintf(tmpname, sizeof(tmpname), "%s.tmp-%ld-%ld", leaf.c_str(), (long)getpid(), (long)random());
    int fd = openat(dirfd, tmpname, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) { int e = errno; close(dirfd); close(basedirfd); errno = e; throw std::runtime_error("Open tmp failed"); }
    size_t off = 0;
    while (off < data.size()) {
        ssize_t w = write(fd, data.data() + off, data.size() - off);
        if (w < 0) { int e = errno; close(fd); unlinkat(dirfd, tmpname, 0); close(dirfd); close(basedirfd); errno = e; throw std::runtime_error("Write failed"); }
        off += (size_t)w;
    }
    fsync(fd);
    close(fd);
    if (renameat(dirfd, tmpname, dirfd, leaf.c_str()) < 0) { int e = errno; unlinkat(dirfd, tmpname, 0); close(dirfd); close(basedirfd); errno = e; throw std::runtime_error("Rename failed"); }
    fsync(dirfd);
    close(dirfd);
    close(basedirfd);
}

int main() {
    try {
        std::string base = "sandbox_cpp";
        mkdir(base.c_str(), 0700);

        // Prepare inputs
        safe_write_input(base, "inputs/empty.bin", {});
        std::vector<uint8_t> hello = {'h','e','l','l','o'};
        safe_write_input(base, "inputs/hello.txt", hello);
        std::vector<uint8_t> kb(1024, 'A');
        safe_write_input(base, "inputs/1kb.bin", kb);
        std::vector<uint8_t> seq(256);
        for (int i = 0; i < 256; ++i) seq[i] = (uint8_t)i;
        safe_write_input(base, "inputs/seq.bin", seq);
        std::vector<uint8_t> zeros(10000, '0');
        safe_write_input(base, "inputs/zeros.txt", zeros);

        // Tests
        Result r1 = process_file(base, "inputs/empty.bin", "outputs/empty.out");
        std::cout << "T1: bytes=" << r1.bytesRead << " checksum=" << r1.checksum << " empty=" << (r1.empty ? "true" : "false") << "\n";
        Result r2 = process_file(base, "inputs/hello.txt", "outputs/hello.out");
        std::cout << "T2: bytes=" << r2.bytesRead << " checksum=" << r2.checksum << " empty=" << (r2.empty ? "true" : "false") << "\n";
        Result r3 = process_file(base, "inputs/1kb.bin", "outputs/1kb.out");
        std::cout << "T3: bytes=" << r3.bytesRead << " checksum=" << r3.checksum << " empty=" << (r3.empty ? "true" : "false") << "\n";
        Result r4 = process_file(base, "inputs/seq.bin", "outputs/seq.out");
        std::cout << "T4: bytes=" << r4.bytesRead << " checksum=" << r4.checksum << " empty=" << (r4.empty ? "true" : "false") << "\n";
        Result r5 = process_file(base, "inputs/zeros.txt", "outputs/zeros.out");
        std::cout << "T5: bytes=" << r5.bytesRead << " checksum=" << r5.checksum << " empty=" << (r5.empty ? "true" : "false") << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}