// Chain-of-Through process:
// 1) Problem understanding: provide a secure function that reads a file and returns its contents; main prints it.
// 2) Security requirements: reject symlinks, ensure regular files, enforce size cap, robust error handling.
// 3) Secure coding generation: use std::filesystem checks, size limit, buffered read, UTF-8 handling as raw bytes to string.
// 4) Code review: validate inputs, exception safety, bounds checks, no undefined behavior.
// 5) Secure code output: final code with mitigations and 5 test cases.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;
static const std::uintmax_t MAX_FILE_SIZE = 10u * 1024u * 1024u; // 10 MB

std::string readFileSecure(const std::string& path) {
    if (path.empty()) {
        throw std::invalid_argument("Invalid path");
    }
    fs::path p = fs::path(path).lexically_normal();

    std::error_code ec;
    if (!fs::exists(p, ec)) {
        throw std::runtime_error("File does not exist");
    }
    if (fs::is_symlink(p, ec)) {
        throw std::runtime_error("Refusing to read symbolic link");
    }
    if (!fs::is_regular_file(p, ec)) {
        throw std::runtime_error("Not a regular file");
    }
    std::uintmax_t size = fs::file_size(p, ec);
    if (ec) {
        throw std::runtime_error("Unable to determine file size");
    }
    if (size > MAX_FILE_SIZE) {
        throw std::runtime_error("File too large");
    }

    std::ifstream in(p, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file");
    }
    std::string data;
    data.reserve(size <= SIZE_MAX ? static_cast<size_t>(size) : 0);
    const size_t BUF_SIZE = 8192;
    std::vector<char> buf(BUF_SIZE);
    std::uintmax_t total = 0;
    while (in) {
        in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        std::streamsize got = in.gcount();
        if (got > 0) {
            total += static_cast<std::uintmax_t>(got);
            if (total > MAX_FILE_SIZE) {
                throw std::runtime_error("File grew beyond size limit while reading");
            }
            data.append(buf.data(), static_cast<size_t>(got));
        }
    }
    return data; // Interpreting as UTF-8 or binary; console prints raw bytes.
}

int main(int argc, char* argv[]) {
    // If a path is provided by user, read and print it.
    if (argc > 1) {
        try {
            std::cout << readFileSecure(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }

    // 5 test cases
    try {
        fs::path dir = fs::temp_directory_path() / "task36_tests_cpp";
        std::error_code ec;
        fs::create_directories(dir, ec);

        // Test 1: small file
        fs::path t1 = dir / "small.txt";
        { std::ofstream o(t1, std::ios::binary); o << "Hello\nWorld\n"; }

        // Test 2: empty file
        fs::path t2 = dir / "empty.txt";
        { std::ofstream o(t2, std::ios::binary); }

        // Test 3: unicode content (UTF-8)
        fs::path t3 = dir / "unicode.txt";
        { std::ofstream o(t3, std::ios::binary); o << u8"こんにちは\n🙂\n"; }

        // Test 4: non-existent file
        fs::path t4 = dir / "missing.txt"; // do not create

        // Test 5: near-limit (1KB)
        fs::path t5 = dir / "limit.txt";
        { std::ofstream o(t5, std::ios::binary); for (int i = 0; i < 1024; ++i) o.put('A'); }

        std::vector<fs::path> tests = { t1, t2, t3, t4, t5 };
        for (const auto& test : tests) {
            std::cout << "\n--- Reading: " << test.string() << " ---\n";
            try {
                std::cout << readFileSecure(test.string());
            } catch (const std::exception& e) {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Test setup error: " << e.what() << "\n";
    }
    return 0;
}