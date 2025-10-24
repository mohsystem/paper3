#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;
static const std::uintmax_t MAX_SIZE = 5u * 1024u * 1024u; // 5MB

static bool isValidFilename(const std::string& s) {
    if (s.empty() || s.size() > 255) return false;
    if (s == "." || s == "..") return false;
    const std::string invalid = "<>:\"/\\|?*";
    for (unsigned char ch : s) {
        if (ch < 32 || ch == 127) return false;
        if (invalid.find(static_cast<char>(ch)) != std::string::npos) return false;
    }
    return true;
}

std::string readFileSecure(const std::string& filename) {
    if (!isValidFilename(filename)) {
        std::cerr << "Invalid filename format." << std::endl;
        return std::string();
    }
    try {
        fs::path p = fs::path(filename).lexically_normal();
        if (!fs::exists(p)) {
            std::cerr << "File does not exist." << std::endl;
            return std::string();
        }
        if (!fs::is_regular_file(p)) {
            std::cerr << "Not a regular file." << std::endl;
            return std::string();
        }
        std::uintmax_t sz = fs::file_size(p);
        if (sz > MAX_SIZE) {
            std::cerr << "File too large." << std::endl;
            return std::string();
        }
        std::ifstream ifs(p, std::ios::binary);
        if (!ifs) {
            std::cerr << "File is not readable." << std::endl;
            return std::string();
        }
        std::string content;
        content.resize(static_cast<size_t>(sz));
        if (sz > 0) {
            ifs.read(&content[0], static_cast<std::streamsize>(sz));
            if (!ifs) {
                std::cerr << "I/O error while reading." << std::endl;
                return std::string();
            }
        }
        return content;
    } catch (const std::exception& e) {
        std::cerr << "I/O error: " << e.what() << std::endl;
        return std::string();
    }
}

static void writeSample(const std::string& name, const std::string& content) {
    try {
        std::ofstream ofs(name, std::ios::binary | std::ios::trunc);
        ofs.write(content.c_str(), static_cast<std::streamsize>(content.size()));
    } catch (...) {
        std::cerr << "Failed to write sample file." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Prepare sample resources
    writeSample("testfile.txt", "Hello from Task65 (C++)!\nThis is a secure read test.\n");
    try {
        fs::create_directories("testdir");
    } catch (...) {
        // ignore
    }

    // 5 test cases
    std::vector<std::string> tests = {
        "testfile.txt",      // valid
        "nonexistent.txt",   // not exist
        "bad|name.txt",      // invalid char
        "../secret.txt",     // traversal attempt (invalid)
        "testdir"            // directory
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "=== C++ Test " << (i + 1) << ": " << tests[i] << " ===" << std::endl;
        std::string data = readFileSecure(tests[i]);
        if (!data.empty()) {
            std::cout << "Content length: " << data.size() << std::endl;
            std::cout << data << std::endl;
        } else {
            std::cout << "Read failed." << std::endl;
        }
    }

    // If an argument is provided, try to read it
    if (argc > 1) {
        std::string res = readFileSecure(argv[1]);
        if (!res.empty()) {
            std::cout << "=== C++ Arg Read ===" << std::endl;
            std::cout << res << std::endl;
        } else {
            std::cout << "Could not read file from argument." << std::endl;
        }
    }

    return 0;
}