// Chain-of-Through process:
// 1) Problem understanding: Implement a function to read a file from a given base directory with a
//    provided filename, also reading from command-line if supplied, with validation and error handling.
// 2) Security requirements: Validate filename format, prevent traversal via restricting characters and
//    ensuring canonical path remains within base, limit file size, handle exceptions.
// 3) Secure coding generation: Use std::regex for validation, std::filesystem for path resolution,
//    file size checks, and safe reading with exceptions disabled.
// 4) Code review: Verified checks for invalid names, existence, regular file, readability (assumed if openable),
//    max-size enforcement, canonical path checks to mitigate symlink escapes.
// 5) Secure code output: Final code adopts safe patterns.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

static const std::size_t MAX_BYTES = 1024; // 1 KiB
static const std::regex SAFE_NAME("^[A-Za-z0-9._-]{1,255}$");

std::string safeReadFile(const std::string& baseDir, const std::string& filename) {
    if (!std::regex_match(filename, SAFE_NAME)) {
        return "ERROR: Invalid filename format. Allowed: letters, digits, . _ - (1..255 chars)";
    }
    try {
        fs::path base = fs::absolute(baseDir).lexically_normal();
        fs::path target = base / filename;

        if (!fs::exists(target)) {
            return "ERROR: File does not exist.";
        }
        if (!fs::is_regular_file(target)) {
            return "ERROR: Not a regular file.";
        }

        // Ensure canonical target remains under base (protect against symlink escape)
        fs::path baseCanon = fs::weakly_canonical(base);
        fs::path targetCanon = fs::canonical(target);
        auto baseStr = baseCanon.string();
        auto targetStr = targetCanon.string();
#ifdef _WIN32
        // Case-insensitive comparison on Windows: convert to lowercase
        std::transform(baseStr.begin(), baseStr.end(), baseStr.begin(), ::tolower);
        std::transform(targetStr.begin(), targetStr.end(), targetStr.begin(), ::tolower);
#endif
        if (targetStr.compare(0, baseStr.size(), baseStr) != 0 ||
            (targetStr.size() > baseStr.size() && targetStr[baseStr.size()] != fs::path::preferred_separator)) {
            // Not within base
            return "ERROR: Unsafe path detected.";
        }

        std::error_code ec;
        auto fsize = fs::file_size(targetCanon, ec);
        if (ec) {
            return std::string("ERROR: Unable to get file size: ") + ec.message();
        }
        if (fsize > MAX_BYTES) {
            std::ostringstream oss;
            oss << "ERROR: File too large (" << fsize << " bytes). Max allowed: " << MAX_BYTES << " bytes.";
            return oss.str();
        }

        std::ifstream in(targetCanon, std::ios::in | std::ios::binary);
        if (!in) {
            return "ERROR: Unable to open file for reading.";
        }
        std::string content;
        content.resize(static_cast<size_t>(fsize));
        if (fsize > 0) {
            in.read(&content[0], static_cast<std::streamsize>(fsize));
            if (!in && !in.eof()) {
                return "ERROR: Failed while reading the file.";
            }
        }
        return content;
    } catch (const std::exception& ex) {
        return std::string("ERROR: ") + ex.what();
    } catch (...) {
        return "ERROR: Unknown error.";
    }
}

int main(int argc, char* argv[]) {
    // Prepare test environment
    fs::path base = fs::temp_directory_path() / fs::path("task65_cpp_") / fs::unique_path();
    fs::create_directories(base);

    // Create test files
    {
        std::ofstream f(base / "sample1.txt", std::ios::binary);
        f << "Hello, Task65!";
    }
    {
        std::ofstream f(base / "sample-2.txt", std::ios::binary);
        f << "Second sample content.";
    }
    {
        std::ofstream f(base / "big.txt", std::ios::binary);
        std::string big(1500, 'A');
        f.write(big.data(), static_cast<std::streamsize>(big.size()));
    }

    std::vector<std::string> tests = {
        "sample1.txt",     // valid
        "sample-2.txt",    // valid
        "../etc/passwd",   // invalid (blocked by format)
        "missing.txt",     // missing
        "big.txt"          // too large
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " (" << tests[i] << "):\n";
        std::cout << safeReadFile(base.string(), tests[i]) << "\n-----\n";
    }

    if (argc > 0 && argc > 1) {
        std::cout << "Command-line read:\n";
        std::cout << safeReadFile(".", argv[1]) << "\n";
    }
    return 0;
}