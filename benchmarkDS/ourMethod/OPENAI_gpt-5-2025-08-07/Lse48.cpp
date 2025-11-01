#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>

static const std::string BASE_DIR = "static";
static const size_t MAX_SIZE = 5 * 1024 * 1024; // 5 MiB

struct ImgResult {
    std::vector<unsigned char> data;
    int status; // 0=ok, nonzero=error
};

static bool valid_name(const std::string& name) {
    if (name.size() < 1 || name.size() > 128) return false;
    for (char c : name) {
        bool ok = (c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static ImgResult img(const std::string& fileParam) {
    ImgResult res;
    res.status = -1;

    if (!valid_name(fileParam)) {
        res.status = EINVAL;
        return res;
    }

    int dirfd = -1;
    int fd = -1;
    do {
        dirfd = open(BASE_DIR.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
        if (dirfd < 0) { res.status = errno ? errno : -1; break; }

        int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
        flags |= O_NOFOLLOW;
#endif
        fd = openat(dirfd, fileParam.c_str(), flags);
        if (fd < 0) { res.status = errno ? errno : -1; break; }

        struct stat st;
        if (fstat(fd, &st) != 0) { res.status = errno ? errno : -1; break; }
        if (!S_ISREG(st.st_mode)) { res.status = EINVAL; break; }
        if (st.st_size < 0 || static_cast<size_t>(st.st_size) > MAX_SIZE) { res.status = EFBIG; break; }

        size_t size = static_cast<size_t>(st.st_size);
        res.data.resize(size);
        size_t off = 0;
        while (off < size) {
            ssize_t r = read(fd, res.data.data() + off, size - off);
            if (r < 0) { res.status = errno ? errno : -1; break; }
            if (r == 0) { res.status = EIO; break; }
            off += static_cast<size_t>(r);
        }
        if (off == size) res.status = 0;
    } while (0);

    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);
    if (res.status != 0) res.data.clear();
    return res;
}

// Helpers for tests
static void ensure_dir(const std::string& path) {
    if (mkdir(path.c_str(), 0700) != 0) {
        if (errno != EEXIST) {
            std::cerr << "mkdir failed: " << strerror(errno) << std::endl;
        }
    }
}

static bool write_file(const std::string& name, const std::vector<unsigned char>& data) {
    ensure_dir(BASE_DIR);
    int dirfd = open(BASE_DIR.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return false;
    int fd = openat(dirfd, name.c_str(), O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) { close(dirfd); return false; }
    size_t off = 0;
    while (off < data.size()) {
        ssize_t w = write(fd, data.data() + off, data.size() - off);
        if (w < 0) { close(fd); close(dirfd); return false; }
        off += static_cast<size_t>(w);
    }
    close(fd);
    close(dirfd);
    return true;
}

static bool write_big(const std::string& name, size_t size) {
    ensure_dir(BASE_DIR);
    int dirfd = open(BASE_DIR.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return false;
    int fd = openat(dirfd, name.c_str(), O_WRONLY | O_CLOEXEC | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) { close(dirfd); return false; }
    if (size > 0) {
        if (lseek(fd, (off_t)size - 1, SEEK_SET) < 0) { close(fd); close(dirfd); return false; }
        if (write(fd, "", 1) != 1) { close(fd); close(dirfd); return false; }
    }
    close(fd);
    close(dirfd);
    return true;
}

int main() {
    ensure_dir(BASE_DIR);
    write_file("hello.txt", std::vector<unsigned char>{'H','e','l','l','o',',',' ','w','o','r','l','d','!'});
    write_file("empty.bin", std::vector<unsigned char>{});
    write_big("big.bin", MAX_SIZE + 1024);

    // Attempt to create a symlink outside
#if defined(__unix__) || defined(__APPLE__)
    std::string linkPath = BASE_DIR + "/link.txt";
    unlink(linkPath.c_str());
    // Ignore errors if not permitted
    symlink("/etc/hosts", linkPath.c_str());
#endif

    // 5 test cases

    // 1) valid small file
    {
        ImgResult r = img("hello.txt");
        if (r.status == 0) std::cout << "Test1 hello.txt size=" << r.data.size() << "\n";
        else std::cout << "Test1 failed: " << r.status << "\n";
    }

    // 2) empty file
    {
        ImgResult r = img("empty.bin");
        if (r.status == 0) std::cout << "Test2 empty.bin size=" << r.data.size() << "\n";
        else std::cout << "Test2 failed: " << r.status << "\n";
    }

    // 3) traversal attempt
    {
        ImgResult r = img("../etc/passwd");
        if (r.status == 0) std::cout << "Test3 traversal size=" << r.data.size() << "\n";
        else std::cout << "Test3 expected failure: " << r.status << "\n";
    }

    // 4) symlink attempt
    {
        ImgResult r = img("link.txt");
        if (r.status == 0) std::cout << "Test4 symlink size=" << r.data.size() << "\n";
        else std::cout << "Test4 expected failure or skipped: " << r.status << "\n";
    }

    // 5) oversized file
    {
        ImgResult r = img("big.bin");
        if (r.status == 0) std::cout << "Test5 big.bin size=" << r.data.size() << "\n";
        else std::cout << "Test5 expected failure: " << r.status << "\n";
    }

    return 0;
}