#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

static const size_t MAX_SIZE = 1024 * 1024; // 1 MiB

static bool validate_rel_path(const std::string& p) {
    if (p.empty() || p.size() > 4096) return false;
    if (p[0] == '/') return false;
    if (p.find('\0') != std::string::npos) return false;
    if (p.find('\\') != std::string::npos) return false;

    size_t i = 0;
    bool seenPart = false;
    while (i < p.size()) {
        size_t j = p.find('/', i);
        std::string seg = (j == std::string::npos) ? p.substr(i) : p.substr(i, j - i);
        if (seg.empty() || seg == "." || seg == "..") return false;
        // Basic charset allowlist
        for (char c : seg) {
            if (!( (c >= 'a' && c <= 'z') ||
                   (c >= 'A' && c <= 'Z') ||
                   (c >= '0' && c <= '9') ||
                   c == '.' || c == '_' || c == '-' )) {
                return false;
            }
        }
        seenPart = true;
        if (j == std::string::npos) break;
        i = j + 1;
    }
    return seenPart;
}

static std::string guess_content_type(const std::string& name) {
    auto idx = name.find_last_of('.');
    std::string ext = (idx == std::string::npos) ? "" : name.substr(idx + 1);
    for (auto& c : ext) c = (char)tolower((unsigned char)c);
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "gif") return "image/gif";
    if (ext == "svg") return "image/svg+xml";
    if (ext == "txt") return "text/plain; charset=utf-8";
    return "application/octet-stream";
}

// Returns HTTP-like status code; on success fills out buffer and content type
static int read_image_secure(const std::string& baseDir, const std::string& relPath, std::vector<uint8_t>& out, std::string& contentType) {
    if (!validate_rel_path(relPath)) {
        return 400;
    }

    int dirfd = open(baseDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) {
        return 500;
    }

    int fd = -1;
    struct stat st;
    memset(&st, 0, sizeof(st));
    int status = 200;

    do {
        int flags = O_RDONLY | O_CLOEXEC;
#ifdef O_NOFOLLOW
        flags |= O_NOFOLLOW;
#endif
        fd = openat(dirfd, relPath.c_str(), flags);
        if (fd < 0) {
            if (errno == ELOOP) {
                status = 404;
            } else if (errno == ENOENT) {
                status = 404;
            } else {
                status = 404;
            }
            break;
        }
        if (fstat(fd, &st) != 0) {
            status = 404;
            break;
        }
        if (!S_ISREG(st.st_mode)) {
            status = 404;
            break;
        }
        if (st.st_size < 0 || (size_t)st.st_size > MAX_SIZE) {
            status = 413;
            break;
        }
        out.clear();
        out.reserve((size_t)st.st_size);
        std::vector<uint8_t> buf(8192);
        ssize_t r;
        while ((r = read(fd, buf.data(), buf.size())) > 0) {
            out.insert(out.end(), buf.begin(), buf.begin() + r);
            if (out.size() > MAX_SIZE) { status = 413; break; }
        }
        if (r < 0) {
            status = 500;
            break;
        }
        if (status != 200) break;
        contentType = guess_content_type(relPath);
        status = 200;
    } while (0);

    if (fd >= 0) close(fd);
    close(dirfd);
    return status;
}

static void write_file(const std::string& path, const std::vector<uint8_t>& data) {
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    ofs.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
}

int main() {
    std::string base = std::string(getenv("PWD") ? getenv("PWD") : ".") + "/images";
    // Prepare images dir
    mkdir(base.c_str(), 0700);

    // hello.txt
    write_file(base + "/hello.txt", std::vector<uint8_t>{'H','e','l','l','o',' ','W','o','r','l','d','!'});

    // big.bin > 1 MiB
    std::vector<uint8_t> big(MAX_SIZE + 1);
    for (size_t i = 0; i < big.size(); ++i) big[i] = (uint8_t)(i & 0xFF);
    write_file(base + "/big.bin", big);

    // outside.txt and symlink inside images
    std::string outside = std::string(getenv("PWD") ? getenv("PWD") : ".") + "/outside.txt";
    write_file(outside, std::vector<uint8_t>{'o','u','t','s','i','d','e'});
#ifdef __unix__
    unlink((base + "/link_to_outside.txt").c_str());
    symlink("../outside.txt", (base + "/link_to_outside.txt").c_str());
#endif

    // 5 test cases
    struct T { std::string rel; int expectMin; int expectMax; };
    std::vector<T> tests = {
        {"hello.txt", 200, 200},
        {"missing.txt", 404, 404},
        {"../outside.txt", 400, 400},
        {"link_to_outside.txt", 400, 404}, // symlink should not be followed
        {"big.bin", 413, 413}
    };

    for (const auto& t : tests) {
        std::vector<uint8_t> out;
        std::string ctype;
        int code = read_image_secure(base, t.rel, out, ctype);
        std::cout << t.rel << " -> " << code << " (len=" << out.size() << ", type=" << ctype << ")\n";
    }
    return 0;
}