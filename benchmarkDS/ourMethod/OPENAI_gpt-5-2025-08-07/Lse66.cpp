#include <sys/types.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <cctype>
#include <cstring>

static const size_t MAX_PDF_SIZE = 10 * 1024 * 1024; // 10 MiB

static bool is_dir_mode(mode_t m) { return S_ISDIR(m); }
static bool is_reg_mode(mode_t m) { return S_ISREG(m); }

static std::string sanitize_filename(const std::string& name) {
    // Extract base name (drop path components)
    std::string base = name;
    size_t pos1 = base.find_last_of('/');
    size_t pos2 = base.find_last_of('\\');
    size_t pos = std::string::npos;
    if (pos1 != std::string::npos && pos2 != std::string::npos) pos = std::max(pos1, pos2);
    else if (pos1 != std::string::npos) pos = pos1;
    else if (pos2 != std::string::npos) pos = pos2;
    if (pos != std::string::npos) base = base.substr(pos + 1);

    std::string out;
    out.reserve(std::min<size_t>(base.size(), 128));
    for (size_t i = 0; i < base.size() && i < 128; ++i) {
        unsigned char ch = static_cast<unsigned char>(base[i]);
        char l = static_cast<char>(std::tolower(ch));
        if ((l >= 'a' && l <= 'z') || (l >= '0' && l <= '9') || l == '.' || l == '_' || l == '-') {
            out.push_back(l);
        } else {
            out.push_back('_');
        }
    }
    if (out.empty()) out = "upload.pdf";
    // ensure .pdf
    auto ends_with = [](const std::string& s, const std::string& suf) {
        return s.size() >= suf.size() && s.compare(s.size() - suf.size(), suf.size(), suf) == 0;
    };
    while (out.find("..") != std::string::npos) {
        size_t idx = out.find("..");
        out.replace(idx, 2, ".");
    }
    if (!ends_with(out, ".pdf")) {
        while (!out.empty() && out.back() == '.') out.pop_back();
        out += ".pdf";
    }
    if (!out.empty() && out[0] == '.') out.insert(out.begin(), 'u');
    return out;
}

static std::string random_hex(size_t bytes = 16) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;
    std::string s;
    s.reserve(bytes * 2);
    for (size_t i = 0; i < bytes / 8 + 1; ++i) {
        unsigned long long v = dis(gen);
        char buf[17];
        std::snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)v);
        s.append(buf);
        if (s.size() >= bytes * 2) break;
    }
    if (s.size() > bytes * 2) s.resize(bytes * 2);
    return s;
}

static std::string save_pdf(const std::vector<unsigned char>& data, const std::string& original_filename, const std::string& base_dir) {
    if (data.size() < 5 || data.size() > MAX_PDF_SIZE) {
        return std::string();
    }
    const char hdr[] = "%PDF-";
    if (std::memcmp(data.data(), hdr, 5) != 0) {
        return std::string();
    }

    int dflags = O_RDONLY;
#ifdef O_DIRECTORY
    dflags |= O_DIRECTORY;
#endif
#ifdef O_CLOEXEC
    dflags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    dflags |= O_NOFOLLOW;
#endif

    int dfd = open(base_dir.c_str(), dflags);
    if (dfd < 0) {
        return std::string();
    }

    struct stat dst;
    if (fstat(dfd, &dst) != 0 || !is_dir_mode(dst.st_mode)) {
        close(dfd);
        return std::string();
    }

    std::string dest = sanitize_filename(original_filename);
    // Only allow filenames without slashes
    if (dest.find('/') != std::string::npos || dest.find('\\') != std::string::npos) {
        close(dfd);
        return std::string();
    }

    std::string tmpname = std::string("upload_") + random_hex(16) + ".tmp";
    int fflags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_CLOEXEC
    fflags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    fflags |= O_NOFOLLOW;
#endif

    int fd = openat(dfd, tmpname.c_str(), fflags, 0600);
    if (fd < 0) {
        close(dfd);
        return std::string();
    }

    // Validate opened is regular file
    struct stat fst;
    if (fstat(fd, &fst) != 0 || !S_ISREG(fst.st_mode)) {
        close(fd);
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }

    // Write data safely
    size_t total = 0;
    const unsigned char* ptr = data.data();
    while (total < data.size()) {
        ssize_t w = write(fd, ptr + total, data.size() - total);
        if (w < 0) {
            close(fd);
            unlinkat(dfd, tmpname.c_str(), 0);
            close(dfd);
            return std::string();
        }
        total += static_cast<size_t>(w);
        if (total > MAX_PDF_SIZE) {
            close(fd);
            unlinkat(dfd, tmpname.c_str(), 0);
            close(dfd);
            return std::string();
        }
    }

    // Flush to disk
    if (fsync(fd) != 0) {
        close(fd);
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }
    close(fd);

    // Atomically link temp to final name (no overwrite), then unlink temp
    if (linkat(dfd, tmpname.c_str(), dfd, dest.c_str(), 0) != 0) {
        unlinkat(dfd, tmpname.c_str(), 0);
        close(dfd);
        return std::string();
    }
    unlinkat(dfd, tmpname.c_str(), 0);

    // Fsync directory best-effort
    fsync(dfd);
    close(dfd);

    // Return absolute-like path base_dir + "/" + dest (not resolving symlinks)
    std::string out = base_dir;
    if (!out.empty() && out.back() != '/') out.push_back('/');
    out += dest;
    return out;
}

int main() {
    // Ensure uploads directory exists
    mkdir("uploads", 0700);

    // Test 1: Valid PDF
    {
        std::vector<unsigned char> pdf;
        std::string s = "%PDF-1.7\n...";
        pdf.assign(s.begin(), s.end());
        std::string p = save_pdf(pdf, "report.pdf", "uploads");
        if (!p.empty()) std::cout << "OK1: " << p << "\n"; else std::cout << "FAIL1\n";
    }

    // Test 2: Path traversal
    {
        std::vector<unsigned char> pdf;
        std::string s = "%PDF-1.7\n...";
        pdf.assign(s.begin(), s.end());
        std::string p = save_pdf(pdf, "../../evil.pdf", "uploads");
        if (!p.empty()) std::cout << "OK2: " << p << "\n"; else std::cout << "FAIL2\n";
    }

    // Test 3: Wrong extension
    {
        std::vector<unsigned char> pdf;
        std::string s = "%PDF-1.7\n...";
        pdf.assign(s.begin(), s.end());
        std::string p = save_pdf(pdf, "image.png", "uploads");
        if (!p.empty()) std::cout << "OK3: " << p << "\n"; else std::cout << "FAIL3\n";
    }

    // Test 4: Too big
    {
        std::vector<unsigned char> big(MAX_PDF_SIZE + 1, 'A');
        big[0] = '%'; big[1] = 'P'; big[2] = 'D'; big[3] = 'F'; big[4] = '-';
        std::string p = save_pdf(big, "big.pdf", "uploads");
        if (!p.empty()) std::cout << "OK4: " << p << "\n"; else std::cout << "FAIL4\n";
    }

    // Test 5: Odd characters
    {
        std::vector<unsigned char> pdf;
        std::string s = "%PDF-1.4\n...";
        pdf.assign(s.begin(), s.end());
        std::string p = save_pdf(pdf, "a..//b??.pdf", "uploads");
        if (!p.empty()) std::cout << "OK5: " << p << "\n"; else std::cout << "FAIL5\n";
    }

    return 0;
}