// Chain-of-Through process:
// 1) Problem understanding: Accept PDF bytes, validate, and save securely in ./uploads, returning the saved path.
// 2) Security requirements: Validate magic header, limit size, ignore user filename for path, safe directory creation, avoid path traversal.
// 3) Secure coding generation: Implement robust checks and safe file writing.
// 4) Code review: In-line comments reflect secure choices.
// 5) Secure code output: Final code below.

#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <random>
#include <stdexcept>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static const size_t MAX_SIZE = 1 * 1024 * 1024; // 1 MiB

static std::filesystem::path ensure_uploads_dir() {
    std::filesystem::path uploads = std::filesystem::absolute("uploads");
    if (!std::filesystem::exists(uploads)) {
        std::filesystem::create_directories(uploads);
#ifndef _WIN32
        ::chmod(uploads.string().c_str(), 0700);
#endif
    }
    return uploads;
}

static bool has_pdf_header(const std::vector<unsigned char>& data) {
    return data.size() >= 5 &&
           data[0] == 0x25 && data[1] == 0x50 &&
           data[2] == 0x44 && data[3] == 0x46 &&
           data[4] == 0x2D; // "%PDF-"
}

static std::string random_hex(size_t bytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    std::string out;
    out.reserve(bytes * 2);
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        unsigned char b = static_cast<unsigned char>(dist(gen));
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

// Returns true on success and fills outPath; throws on validation errors.
bool savePdf(const std::vector<unsigned char>& data, const std::string& originalFilename, std::string& outPath) {
    if (data.size() < 5) throw std::invalid_argument("data too small to be a PDF");
    if (data.size() > MAX_SIZE) throw std::invalid_argument("file too large");
    if (!has_pdf_header(data)) throw std::invalid_argument("invalid PDF header");

    auto uploads = ensure_uploads_dir();

    // Ignore original filename; generate random name
    std::string fname = "pdf-" + random_hex(12) + ".pdf";
    std::filesystem::path target = std::filesystem::weakly_canonical(uploads / fname);

    // Ensure target is under uploads
    if (target.string().rfind(uploads.string(), 0) != 0) {
        throw std::runtime_error("Invalid path resolution");
    }

#ifdef _WIN32
    // On Windows, use _sopen_s with _O_CREAT | _O_EXCL for exclusive creation
    int fd;
    int rc = _sopen_s(&fd, target.string().c_str(), _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if (rc != 0) {
        throw std::runtime_error("failed to create file");
    }
    FILE* f = _fdopen(fd, "wb");
    if (!f) {
        _close(fd);
        throw std::runtime_error("fdopen failed");
    }
    size_t written = fwrite(data.data(), 1, data.size(), f);
    fclose(f);
    if (written != data.size()) {
        std::filesystem::remove(target);
        throw std::runtime_error("short write");
    }
#else
    int fd = ::open(target.string().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) {
        throw std::runtime_error("failed to create file");
    }
    ssize_t off = 0;
    while (off < static_cast<ssize_t>(data.size())) {
        ssize_t n = ::write(fd, data.data() + off, data.size() - off);
        if (n <= 0) {
            ::close(fd);
            std::filesystem::remove(target);
            throw std::runtime_error("write failed");
        }
        off += n;
    }
    ::close(fd);
#endif

    outPath = target.string();
    return true;
}

int main() {
    // 1) Valid small PDF
    try {
        std::string s = "%PDF-1.4\n%%EOF";
        std::vector<unsigned char> pdf1(s.begin(), s.end());
        std::string out;
        bool ok = savePdf(pdf1, "test1.pdf", out);
        std::cout << "Test1 OK: " << ok << " " << out << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test1 FAILED: " << e.what() << "\n";
    }

    // 2) Invalid header
    try {
        std::string s = "HELLO";
        std::vector<unsigned char> notPdf(s.begin(), s.end());
        std::string out;
        bool ok = savePdf(notPdf, "x.pdf", out);
        std::cout << "Test2 FAILED: " << ok << " " << out << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test2 OK (rejected): " << e.what() << "\n";
    }

    // 3) Oversized
    try {
        std::vector<unsigned char> big(MAX_SIZE + 10, 0);
        big[0] = 0x25; big[1] = 0x50; big[2] = 0x44; big[3] = 0x46; big[4] = 0x2D; // "%PDF-"
        std::string out;
        bool ok = savePdf(big, "big.pdf", out);
        std::cout << "Test3 FAILED: " << ok << " " << out << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test3 OK (rejected): " << e.what() << "\n";
    }

    // 4) Path traversal (ignored for saving)
    try {
        std::string s = "%PDF-1.7\n%%EOF";
        std::vector<unsigned char> pdf4(s.begin(), s.end());
        std::string out;
        bool ok = savePdf(pdf4, "../../evil.pdf", out);
        std::cout << "Test4 OK: " << ok << " " << out << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test4 FAILED: " << e.what() << "\n";
    }

    // 5) Too small data
    try {
        std::vector<unsigned char> tiny{0x25, 0x50, 0x44, 0x46}; // "%PDF" missing '-'
        std::string out;
        bool ok = savePdf(tiny, "tiny.pdf", out);
        std::cout << "Test5 FAILED: " << ok << " " << out << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test5 OK (rejected): " << e.what() << "\n";
    }

    return 0;
}