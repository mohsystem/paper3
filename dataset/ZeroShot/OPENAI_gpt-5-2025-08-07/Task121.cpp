#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <random>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static const size_t MAX_SIZE_BYTES = 5ull * 1024ull * 1024ull;
static const std::vector<std::string> ALLOWED_EXTENSIONS = {"txt","pdf","png","jpg","jpeg","gif"};
static const std::string UPLOAD_DIR = "uploads";

static bool is_allowed_ext(const std::string &ext) {
    for (const auto &e : ALLOWED_EXTENSIONS) {
        if (ext == e) return true;
    }
    return false;
}

static std::pair<std::string,std::string> sanitize_filename(const std::string &name) {
    if (name.find("..") != std::string::npos || name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
        throw std::runtime_error("Invalid filename (path traversal detected).");
    }
    std::string base;
    for (char c : name) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') base.push_back(c);
    }
    while (!base.empty() && base.front() == '.') base.erase(base.begin());
    if (base.size() > 255) base = base.substr(base.size() - 255);
    auto pos = base.find_last_of('.');
    if (pos == std::string::npos || pos == 0 || pos == base.size() - 1) {
        throw std::runtime_error("Filename must include an extension.");
    }
    std::string stem = base.substr(0, pos);
    std::string ext = base.substr(pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext.size() == 0 || ext.size() > 10) throw std::runtime_error("Invalid extension length.");
    if (!is_allowed_ext(ext)) throw std::runtime_error("Disallowed file extension.");
    return {stem, ext};
}

static std::string random_hex(size_t bytes) {
    std::random_device rd;
    std::string out;
    out.reserve(bytes * 2);
    for (size_t i = 0; i < bytes; ++i) {
        unsigned int v = rd() & 0xFF;
        const char *hex = "0123456789abcdef";
        out.push_back(hex[(v >> 4) & 0xF]);
        out.push_back(hex[v & 0xF]);
    }
    return out;
}

static void ensure_upload_dir() {
    std::filesystem::path p(UPLOAD_DIR);
    if (!std::filesystem::exists(p)) {
        std::filesystem::create_directories(p);
    }
#if defined(__unix__) || defined(__APPLE__)
    std::filesystem::permissions(p,
        std::filesystem::perms::owner_read | std::filesystem::perms::owner_write | std::filesystem::perms::owner_exec,
        std::filesystem::perm_options::replace);
#endif
}

std::string upload_file(const std::string &original_name, const std::vector<uint8_t> &content) {
    if (original_name.empty() || content.empty()) {
        throw std::runtime_error("Filename and content must not be empty.");
    }
    if (content.size() > MAX_SIZE_BYTES) {
        throw std::runtime_error("File exceeds maximum allowed size.");
    }

    auto [stem, ext] = sanitize_filename(original_name);
    ensure_upload_dir();

    std::string unique = random_hex(16) + "." + ext;
    std::filesystem::path uploadDir = std::filesystem::absolute(UPLOAD_DIR);
    std::filesystem::path dest = (uploadDir / unique);
    dest = std::filesystem::weakly_canonical(dest);
    if (dest.string().rfind(uploadDir.string(), 0) != 0) {
        throw std::runtime_error("Resolved path escapes upload directory.");
    }

#if defined(__unix__) || defined(__APPLE__)
    int fd = ::open(dest.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd == -1) {
        throw std::runtime_error("Failed to create destination file.");
    }
    size_t total = 0;
    while (total < content.size()) {
        ssize_t w = ::write(fd, content.data() + total, content.size() - total);
        if (w <= 0) {
            ::close(fd);
            throw std::runtime_error("Write failed.");
        }
        total += static_cast<size_t>(w);
    }
    ::close(fd);
#else
    // Fallback for non-POSIX (e.g., Windows)
    if (std::filesystem::exists(dest)) {
        throw std::runtime_error("Name collision.");
    }
    std::ofstream ofs(dest, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open file for writing.");
    }
    ofs.write(reinterpret_cast<const char*>(content.data()), static_cast<std::streamsize>(content.size()));
    if (!ofs) {
        throw std::runtime_error("Write failed.");
    }
    ofs.close();
#endif
    return dest.string();
}

int main() {
    try {
        std::vector<std::pair<std::string, std::vector<uint8_t>>> tests;
        tests.push_back({"hello.txt", std::vector<uint8_t>{'H','e','l','l','o',',',' ','W','o','r','l','d','!'}});
        tests.push_back({"image.jpg", std::vector<uint8_t>{0xFF,0xD8,0xFF,0xD9}});
        tests.push_back({"report.pdf", std::vector<uint8_t>{'%', 'P','D','F','-','1','.','4','\n','%', 0xE2, 0xE3, 0xCF, 0xD3, '\n'}});
        tests.push_back({"evil/../secret.txt", std::vector<uint8_t>{'t','o','p',' ','s','e','c','r','e','t'}});
        tests.push_back({"malware.exe", std::vector<uint8_t>{'M','Z',0,0}});
        for (auto &t : tests) {
            try {
                std::string saved = upload_file(t.first, t.second);
                std::cout << "Saved: " << saved << "\n";
            } catch (const std::exception &e) {
                std::cout << "Failed: " << t.first << " -> " << e.what() << "\n";
            }
        }
    } catch (const std::exception &e) {
        std::cout << "Fatal error: " << e.what() << "\n";
    }
    return 0;
}