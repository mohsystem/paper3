#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <random>

static const size_t MAX_IMAGE_SIZE = 1 * 1024 * 1024; // 1 MiB
static const size_t NAME_MAX_LEN = 255;

struct Record {
    int id;
    std::string name;
    std::string b64;
};

class Database {
public:
    int insertImage(const std::string& name, const std::string& b64) {
        if (!isSafeSimpleName(name)) throw std::runtime_error("Bad name");
        if (b64.empty() || b64.size() > MAX_IMAGE_SIZE * 2) throw std::runtime_error("Data too large");
        int id = ++nextId_;
        records_.push_back(Record{id, name, b64});
        return id;
    }
    size_t count() const { return records_.size(); }
    const std::vector<Record>& all() const { return records_; }
private:
    static bool isSafeSimpleName(const std::string& s) {
        if (s.empty() || s.size() > NAME_MAX_LEN) return false;
        for (unsigned char c : s) {
            bool ok = (c >= 'A' && c <= 'Z') ||
                      (c >= 'a' && c <= 'z') ||
                      (c >= '0' && c <= '9') ||
                      c == '.' || c == '_' || c == '-';
            if (!ok) return false;
        }
        return true;
    }
    std::vector<Record> records_;
    int nextId_{0};
};

static bool isSafeSimpleName(const std::string& s) {
    if (s.empty() || s.size() > NAME_MAX_LEN) return false;
    for (unsigned char c : s) {
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static std::string base64_encode(const std::vector<unsigned char>& data) {
    static const char* tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);
    size_t i = 0;
    while (i + 3 <= data.size()) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i + 1 == data.size()) {
        unsigned int n = (data[i] << 16);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back('=');
        out.push_back('=');
    } else if (i + 2 == data.size()) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back('=');
    }
    return out;
}

// Securely open baseDir (as dirfd), then open relative file name with O_NOFOLLOW, read, base64, insert.
bool upload_image(const std::string& baseDir, const std::string& relativeFileName, const std::string& imageName, Database& db) {
    if (!isSafeSimpleName(relativeFileName) || !isSafeSimpleName(imageName)) {
        std::cout << "Rejected: invalid name\n";
        return false;
    }

    int dirfd = open(baseDir.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) {
        std::cout << "Failed to open base dir: " << std::strerror(errno) << "\n";
        return false;
    }

    int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
    int fd = openat(dirfd, relativeFileName.c_str(), flags);
    if (fd < 0) {
        std::cout << "Failed to open file: " << std::strerror(errno) << "\n";
        close(dirfd);
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        std::cout << "Failed to fstat: " << std::strerror(errno) << "\n";
        close(fd); close(dirfd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        std::cout << "Rejected: not a regular file\n";
        close(fd); close(dirfd);
        return false;
    }
    if (static_cast<size_t>(st.st_size) > MAX_IMAGE_SIZE) {
        std::cout << "Rejected: file too large\n";
        close(fd); close(dirfd);
        return false;
    }

    std::vector<unsigned char> data;
    data.reserve(static_cast<size_t>(st.st_size));
    unsigned char buf[8192];
    size_t total = 0;
    while (true) {
        ssize_t n = read(fd, buf, sizeof(buf));
        if (n < 0) {
            std::cout << "Read error: " << std::strerror(errno) << "\n";
            close(fd); close(dirfd);
            return false;
        }
        if (n == 0) break;
        total += static_cast<size_t>(n);
        if (total > MAX_IMAGE_SIZE) {
            std::cout << "Rejected: file too large (during read)\n";
            close(fd); close(dirfd);
            return false;
        }
        data.insert(data.end(), buf, buf + n);
    }

    std::string b64 = base64_encode(data);
    try {
        db.insertImage(imageName, b64);
    } catch (const std::exception& ex) {
        std::cout << "DB insert failed: " << ex.what() << "\n";
        close(fd); close(dirfd);
        return false;
    }

    close(fd);
    close(dirfd);
    return true;
}

static std::string mktempdir() {
    std::string tmpl = "/tmp/img_upload_cpp_XXXXXX";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    char* res = mkdtemp(buf.data());
    if (!res) {
        throw std::runtime_error("mkdtemp failed");
    }
    return std::string(res);
}

static bool write_file(const std::string& baseDir, const std::string& name, size_t size) {
    int dirfd = open(baseDir.c_str(), O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) return false;
    int fd = openat(dirfd, name.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
    if (fd < 0) { close(dirfd); return false; }
    std::vector<unsigned char> chunk(4096);
    std::random_device rd; std::mt19937 gen(rd()); std::uniform_int_distribution<int> dist(0, 255);
    for (auto& c : chunk) c = static_cast<unsigned char>(dist(gen));

    size_t remaining = size;
    while (remaining > 0) {
        size_t w = remaining < chunk.size() ? remaining : chunk.size();
        ssize_t n = write(fd, chunk.data(), w);
        if (n < 0) { close(fd); close(dirfd); return false; }
        remaining -= static_cast<size_t>(n);
    }
    fsync(fd);
    close(fd);
    close(dirfd);
    return true;
}

int main() {
    try {
        Database db;
        std::string baseDir = mktempdir();
        std::cout << "Base directory: " << baseDir << "\n";

        write_file(baseDir, "t1.bin", 1024);
        write_file(baseDir, "t2.bin", 2048);
        write_file(baseDir, "t3.bin", 512);
        write_file(baseDir, "big.bin", 2 * 1024 * 1024); // oversize

        bool r1 = upload_image(baseDir, "t1.bin", "image_one.png", db);
        std::cout << "Test1 (valid): " << (r1 ? "true" : "false") << "\n";

        bool r2 = upload_image(baseDir, "t2.bin", "image-two.jpg", db);
        std::cout << "Test2 (valid): " << (r2 ? "true" : "false") << "\n";

        bool r3 = upload_image(baseDir, "big.bin", "too_big.dat", db);
        std::cout << "Test3 (oversize): " << (r3 ? "true" : "false") << "\n";

        bool r4 = upload_image(baseDir, "t_missing.bin", "missing.gif", db);
        std::cout << "Test4 (missing): " << (r4 ? "true" : "false") << "\n";

        bool r5 = upload_image(baseDir, "t3.bin", "safe_name-ok_123.bmp", db);
        std::cout << "Test5 (valid): " << (r5 ? "true" : "false") << "\n";

        std::cout << "DB count: " << db.count() << "\n";
        for (const auto& rec : db.all()) {
            std::cout << "Record " << rec.id << " name=" << rec.name << " b64_len=" << rec.b64.size() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cout << "Fatal: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}