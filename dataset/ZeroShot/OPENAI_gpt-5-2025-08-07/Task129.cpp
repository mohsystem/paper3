#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

static const uintmax_t MAX_FILE_SIZE = 1048576; // 1 MB

static bool isValidFilename(const std::string& name) {
    static const std::regex re("^[A-Za-z0-9_.-]{1,128}$");
    if (!std::regex_match(name, re)) return false;
    if (!name.empty() && name[0] == '.') return false;

    auto dotPos = name.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == name.size() - 1) return false;
    std::string ext = name.substr(dotPos + 1);
    for (auto& c : ext) c = static_cast<char>(::tolower(c));
    static const std::unordered_set<std::string> allowed = {"txt", "json", "csv"};
    return allowed.count(ext) > 0;
}

std::string fetchFile(const std::string& baseDir, const std::string& filename) {
    if (baseDir.empty() || filename.empty()) {
        throw std::runtime_error("Invalid arguments");
    }
    if (!isValidFilename(filename)) {
        throw std::runtime_error("Invalid filename");
    }

    fs::path base = fs::canonical(fs::path(baseDir));
    fs::path rel = fs::path(filename);
    if (rel.is_absolute()) {
        throw std::runtime_error("Absolute path not allowed");
    }
    fs::path combined = (base / rel).lexically_normal();

    if (!fs::exists(combined) || !fs::is_regular_file(combined)) {
        throw std::runtime_error("File not found");
    }

    fs::path real = fs::canonical(combined);

    // Ensure real path starts with base path (component-wise)
    auto baseIt = base.begin(), baseEnd = base.end();
    auto realIt = real.begin(), realEnd = real.end();
    for (; baseIt != baseEnd && realIt != realEnd; ++baseIt, ++realIt) {
        if (*baseIt != *realIt) {
            throw std::runtime_error("Access denied");
        }
    }
    if (baseIt != baseEnd) {
        throw std::runtime_error("Access denied");
    }

    uintmax_t size = fs::file_size(real);
    if (size > MAX_FILE_SIZE) {
        throw std::runtime_error("File too large");
    }

    std::ifstream ifs(real, std::ios::binary);
    if (!ifs) throw std::runtime_error("Open failed");
    std::string content;
    content.resize(static_cast<size_t>(size));
    if (size > 0) {
        ifs.read(&content[0], static_cast<std::streamsize>(size));
    }
    return content;
}

int main() {
    try {
        fs::path base = fs::temp_directory_path() / "task129_cpp_base";
        fs::create_directories(base);
        // Create files
        {
            std::ofstream(base / "a.txt") << "Hello from C++\n";
            std::ofstream(base / "data.json") << "{\"k\":\"v\"}\n";
            std::ofstream(base / "doc.csv") << "c1,c2\n1,2\n";
            std::ofstream big(base / "big.txt", std::ios::binary);
            std::string chunk(1024, 'A');
            for (size_t i = 0; i < (MAX_FILE_SIZE / 1024) + 2; ++i) big.write(chunk.data(), chunk.size());
        }

        std::vector<std::string> tests = {
            "a.txt",          // valid
            "data.json",      // valid
            "../etc/passwd",  // traversal
            "secret.conf",    // invalid extension
            "big.txt"         // too large
        };

        for (const auto& t : tests) {
            try {
                std::string content = fetchFile(base.string(), t);
                for (auto& ch : content) { if (ch == '\n') ch = '\\'; } // crude print-safe transform
                std::cout << "FETCH OK [" << t << "]: " << content << "\n";
            } catch (const std::exception& ex) {
                std::cout << "FETCH FAIL [" << t << "]: " << ex.what() << "\n";
            }
        }

        // Cleanup best-effort
        for (auto& p : fs::directory_iterator(base)) {
            std::error_code ec;
            fs::remove(p.path(), ec);
        }
        std::error_code ec;
        fs::remove(base, ec);
    } catch (const std::exception& ex) {
        std::cout << "Setup error: " << ex.what() << "\n";
    }
    return 0;
}