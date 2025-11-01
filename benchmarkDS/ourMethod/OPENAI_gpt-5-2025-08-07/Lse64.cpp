#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <regex>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static const size_t MAX_UPLOAD_SIZE = 32 * 1024 * 1024;

static std::string sanitize_filename(const std::string& name) {
    std::string base = name;
    // strip directories
    size_t pos = base.find_last_of("/\\");
    if (pos != std::string::npos) base = base.substr(pos + 1);
    // remove NULs
    base.erase(std::remove(base.begin(), base.end(), '\0'), base.end());
    // allow only [A-Za-z0-9._-]
    for (char& c : base) {
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-')) {
            c = '_';
        }
    }
    // ensure .pdf
    auto lower = base;
    for (char& c : lower) c = (char)std::tolower((unsigned char)c);
    if (lower.size() < 4 || lower.substr(lower.size() - 4) != ".pdf") {
        // strip last extension
        size_t dot = base.find_last_of('.');
        if (dot != std::string::npos) base = base.substr(0, dot);
        base += ".pdf";
    }
    if (base.size() > 100) base = base.substr(base.size() - 100);
    return base;
}

static void fsync_dir_fd(int dirfd) {
    if (dirfd >= 0) {
        (void)fsync(dirfd);
    }
}

static void ensure_uploads(const std::string& base_dir) {
    struct stat st;
    if (stat(base_dir.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            if (mkdir(base_dir.c_str(), 0700) != 0 && errno != EEXIST) {
                throw std::runtime_error("failed to create base dir");
            }
        } else {
            throw std::runtime_error("stat base dir failed");
        }
    }
    if (stat(base_dir.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        throw std::runtime_error("base dir not directory");
    }
    std::string uploads = base_dir + "/uploads";
    if (stat(uploads.c_str(), &st) != 0) {
        if (errno == ENOENT) {
            if (mkdir(uploads.c_str(), 0700) != 0 && errno != EEXIST) {
                throw std::runtime_error("failed to create uploads");
            }
        }
    }
    if (stat(uploads.c_str(), &st) != 0 || !S_ISDIR(st.st_mode)) {
        throw std::runtime_error("uploads not directory");
    }
    // best-effort: refuse if uploads is symlink
    struct stat lst;
    if (lstat(uploads.c_str(), &lst) == 0 && S_ISLNK(lst.st_mode)) {
        throw std::runtime_error("uploads must not be symlink");
    }
}

static void store_path_mysql_mock(const std::string& base_dir, const std::string& rel_path) {
    std::string idx = base_dir + "/mock_mysql_index.txt";
    FILE* f = fopen(idx.c_str(), "ab");
    if (!f) return;
    fwrite(rel_path.c_str(), 1, rel_path.size(), f);
    fwrite("\n", 1, 1, f);
    fclose(f);
}

static std::string upload_pdf(const std::string& base_dir,
                              const std::string& original_filename,
                              const std::vector<uint8_t>& content) {
    if (content.empty()) throw std::runtime_error("empty content");
    if (content.size() > MAX_UPLOAD_SIZE) throw std::runtime_error("file too large");
    ensure_uploads(base_dir);

    int basefd = open(base_dir.c_str(), O_RDONLY | O_CLOEXEC);
    if (basefd < 0) throw std::runtime_error("open base dir failed");
    int ufd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (ufd < 0) {
        close(basefd);
        throw std::runtime_error("open uploads dir failed");
    }
    struct stat st;
    if (fstat(ufd, &st) != 0 || !S_ISDIR(st.st_mode)) {
        close(ufd); close(basefd);
        throw std::runtime_error("uploads not directory");
    }

    std::string safe = sanitize_filename(original_filename);
    // random prefix
    uint64_t r;
    {
        FILE* ur = fopen("/dev/urandom", "rb");
        if (ur && fread(&r, 1, sizeof(r), ur) == sizeof(r)) {
            fclose(ur);
        } else {
            if (ur) fclose(ur);
            r = (uint64_t)rand() << 32 ^ (uint64_t)rand();
        }
    }
    char prefix[17]; snprintf(prefix, sizeof(prefix), "%016llx", (unsigned long long)r);
    std::string final_name = std::string(prefix).substr(0, 12) + "_" + safe;
    std::string temp_name = final_name + ".part_" + std::string(prefix).substr(12, 4);

    int fd = openat(ufd, temp_name.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) {
        close(ufd); close(basefd);
        throw std::runtime_error("create temp failed");
    }

    size_t written = 0;
    while (written < content.size()) {
        size_t chunk = content.size() - written;
        if (chunk > 65536) chunk = 65536;
        ssize_t n = write(fd, content.data() + written, chunk);
        if (n <= 0) {
            int e = errno;
            close(fd); unlinkat(ufd, temp_name.c_str(), 0);
            close(ufd); close(basefd);
            throw std::runtime_error(std::string("write failed: ") + strerror(e));
        }
        written += (size_t)n;
    }
    fsync(fd);
    close(fd);

    // Atomic rename
    if (renameat(ufd, temp_name.c_str(), ufd, final_name.c_str()) != 0) {
        unlinkat(ufd, temp_name.c_str(), 0);
        close(ufd); close(basefd);
        throw std::runtime_error("rename failed");
    }
    fsync_dir_fd(ufd);

    std::string rel = "uploads/" + final_name;
    // Mock DB store so example runs without MySQL client libs
    store_path_mysql_mock(base_dir, rel);

    close(ufd);
    close(basefd);
    return rel;
}

static std::vector<uint8_t> download_pdf(const std::string& base_dir, const std::string& rel_path) {
    if (rel_path.empty() || rel_path.find("..") != std::string::npos || rel_path[0] == '/')
        throw std::runtime_error("invalid stored path");
    if (rel_path.rfind("uploads/", 0) != 0)
        throw std::runtime_error("must be within uploads/");

    int basefd = open(base_dir.c_str(), O_RDONLY | O_CLOEXEC);
    if (basefd < 0) throw std::runtime_error("open base dir failed");
    int ufd = openat(basefd, "uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (ufd < 0) { close(basefd); throw std::runtime_error("open uploads failed"); }

    std::string fname = rel_path.substr(std::string("uploads/").size());
    int fd = openat(ufd, fname.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) { close(ufd); close(basefd); throw std::runtime_error("open file failed"); }
    struct stat st;
    if (fstat(fd, &st) != 0) { close(fd); close(ufd); close(basefd); throw std::runtime_error("fstat failed"); }
    if (!S_ISREG(st.st_mode)) { close(fd); close(ufd); close(basefd); throw std::runtime_error("not regular file"); }
    if (st.st_size < 0 || (size_t)st.st_size > MAX_UPLOAD_SIZE) {
        close(fd); close(ufd); close(basefd); throw std::runtime_error("invalid size");
    }
    std::vector<uint8_t> out((size_t)st.st_size);
    size_t off = 0;
    while (off < out.size()) {
        ssize_t n = read(fd, out.data() + off, out.size() - off);
        if (n <= 0) break;
        off += (size_t)n;
    }
    close(fd);
    close(ufd);
    close(basefd);
    return out;
}

static std::vector<uint8_t> sample_pdf_bytes(const std::string& title) {
    std::string pdf = "%PDF-1.4\n"
                      "1 0 obj <</Type /Catalog /Pages 2 0 R>> endobj\n"
                      "2 0 obj <</Type /Pages /Kids [3 0 R] /Count 1>> endobj\n"
                      "3 0 obj <</Type /Page /Parent 2 0 R /MediaBox [0 0 200 200] /Contents 4 0 R>> endobj\n"
                      "4 0 obj <</Length 44>> stream\n"
                      "BT /F1 12 Tf 72 120 Td (" + title + ") Tj ET\n"
                      "endstream endobj\n"
                      "xref\n0 5\n0000000000 65535 f \n"
                      "trailer <</Root 1 0 R>>\nstartxref\n0\n%%EOF\n";
    return std::vector<uint8_t>(pdf.begin(), pdf.end());
}

int main() {
    std::string base = "secure_base_cpp";
    std::vector<std::string> tests = {
        "report.pdf",
        "../evil.pdf",
        "weird name.PDF",
        "semi;colon.pdf",
        "a/bad/path.pdf"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            auto data = sample_pdf_bytes("Test " + std::to_string(i + 1));
            std::string stored = upload_pdf(base, tests[i], data);
            auto back = download_pdf(base, stored);
            std::cout << "OK: " << tests[i] << " -> " << stored << " size=" << back.size() << "\n";
        } catch (const std::exception& ex) {
            std::cout << "FAIL: " << tests[i] << " -> " << ex.what() << "\n";
        }
    }
    return 0;
}