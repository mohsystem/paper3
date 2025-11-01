#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string readFromSafeDir(const std::string& filename) {
    if (filename.empty() || filename.find('\0') != std::string::npos) {
        throw std::invalid_argument("Invalid filename");
    }

    fs::path base("/safe");
    fs::path baseCanon;
    try {
        baseCanon = fs::canonical(base);
    } catch (...) {
        throw std::runtime_error("Safe directory not found");
    }

    fs::path input(filename);
    if (input.is_absolute()) {
        throw std::runtime_error("Absolute paths are not allowed");
    }

    fs::path requested = (baseCanon / input).lexically_normal();

    // Ensure requested is inside base
    auto baseStr = baseCanon.string();
    auto reqStr = requested.string();
#ifdef _WIN32
    // Case-insensitive on Windows; but path is Unix-style as per prompt
#endif
    if (reqStr.size() < baseStr.size() || reqStr.compare(0, baseStr.size(), baseStr) != 0 ||
        (reqStr.size() > baseStr.size() && reqStr[baseStr.size()] != fs::path::preferred_separator)) {
        throw std::runtime_error("Path escapes safe directory");
    }

    // Disallow symlinks in the path (including final component)
    fs::path rel = fs::relative(requested, baseCanon);
    if (rel.empty()) {
        throw std::runtime_error("Invalid target");
    }
    fs::path cur = baseCanon;
    for (const auto& part : rel) {
        cur /= part;
        std::error_code ec;
        if (fs::is_symlink(cur, ec)) {
            throw std::runtime_error("Symlinks are not allowed in path");
        }
        if (ec) {
            throw std::runtime_error("Filesystem error");
        }
    }

    std::error_code ec;
    if (!fs::is_regular_file(requested, ec) || ec) {
        throw std::runtime_error("Not a regular file");
    }

    const std::uintmax_t maxBytes = 10u * 1024u * 1024u; // 10 MB
    std::uintmax_t size = fs::file_size(requested, ec);
    if (ec) throw std::runtime_error("Could not get file size");
    if (size > maxBytes) throw std::runtime_error("File too large");

    std::ifstream in(requested, std::ios::in | std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open file");

    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main() {
    std::vector<std::string> tests = {
        "example.txt",
        "../etc/passwd",
        "/etc/passwd",
        "subdir/notes.txt",
        "symlink_to_secret"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        const auto& t = tests[i];
        std::cout << "Test " << (i + 1) << " (" << t << "):" << std::endl;
        try {
            std::string content = readFromSafeDir(t);
            if (content.size() > 200) {
                std::cout << content.substr(0, 200) << "..." << std::endl;
            } else {
                std::cout << content << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}