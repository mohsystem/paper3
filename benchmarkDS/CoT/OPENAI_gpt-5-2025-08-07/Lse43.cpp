// Chain-of-Through:
// 1) Purpose: safely read a file under /safe directory (or provided base).
// 2) Security: sanitize filename, prevent traversal, ensure path within base, reject symlinks, limit size.
// 3) Implementation: use std::filesystem with careful checks; no symlink follow; size capped.
// 4) Review: check each step and error handling.
// 5) Output: secure final code.

#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

static const std::string FIXED_BASE_DIR = "/safe";
static const uintmax_t MAX_BYTES = 10u * 1024u * 1024u; // 10 MB
static const std::regex FILENAME_RE("^[A-Za-z0-9._-]{1,255}$");

std::string readFromBase(const std::string& baseDir, const std::string& filename) {
    if (baseDir.empty()) throw std::invalid_argument("Base directory must not be empty");
    if (filename.empty()) throw std::invalid_argument("Filename must not be empty");
    if (!std::regex_match(filename, FILENAME_RE)) {
        throw std::runtime_error("Invalid filename: only alphanumerics, dot, underscore, and hyphen allowed");
    }

    std::error_code ec;
    fs::path base = fs::absolute(fs::path(baseDir)).lexically_normal();
    fs::path target = (base / filename).lexically_normal();

    // Ensure target is inside base
    auto baseStr = base.string();
    auto targetStr = target.string();
#ifdef _WIN32
    // Normalize case-insensitive if needed; for brevity we rely on lexically_normal and prefix check
#endif
    if (targetStr.size() < baseStr.size() || targetStr.compare(0, baseStr.size(), baseStr) != 0 ||
        (targetStr.size() > baseStr.size() && targetStr[baseStr.size()] != fs::path::preferred_separator)) {
        throw std::runtime_error("Attempt to access outside base directory");
    }

    // Reject symlinks
    fs::file_status syms = fs::symlink_status(target, ec);
    if (ec) throw std::runtime_error("Filesystem error checking symlink status");
    if (fs::is_symlink(syms)) throw std::runtime_error("Symbolic links are not allowed");

    // Must be regular file
    fs::file_status st = fs::status(target, ec);
    if (ec || !fs::is_regular_file(st)) {
        throw std::runtime_error("File does not exist or is not a regular file");
    }

    // Size check
    uintmax_t sz = fs::file_size(target, ec);
    if (ec) throw std::runtime_error("Unable to get file size");
    if (sz > MAX_BYTES) throw std::runtime_error("File too large");

    // Read file
    std::ifstream in(target, std::ios::binary);
    if (!in) throw std::runtime_error("Failed to open file");

    std::string data;
    data.reserve(static_cast<size_t>(std::min<uintmax_t>(sz, static_cast<uintmax_t>(SIZE_MAX))));
    const size_t BUF = 8192;
    char buf[BUF];
    uintmax_t total = 0;
    while (in) {
        in.read(buf, BUF);
        std::streamsize got = in.gcount();
        if (got > 0) {
            total += static_cast<uintmax_t>(got);
            if (total > MAX_BYTES) throw std::runtime_error("File exceeded maximum allowed size while reading");
            data.append(buf, static_cast<size_t>(got));
        }
    }
    return data; // raw bytes; caller may treat as UTF-8 text
}

std::string readFromSafeDir(const std::string& filename) {
    return readFromBase(FIXED_BASE_DIR, filename);
}

// Test helper
void printResult(const std::string& label, const std::function<std::string()>& fn) {
    std::cout << "== " << label << " ==" << std::endl;
    try {
        std::string out = fn();
        if (out.empty()) std::cout << "(empty)" << std::endl;
        else std::cout << out << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Prepare temp base
    fs::path tempBase = fs::temp_directory_path() / "safe_cpp_tests";
    std::error_code ec;
    fs::create_directories(tempBase, ec);

    std::ofstream(tempBase / "hello.txt") << "Hello from C++";
    fs::create_directories(tempBase / "not_a_file", ec);

    std::cout << "Base for tests: " << tempBase << std::endl;

    // Test 1: Valid
    printResult("Test 1: valid read", [&](){ return readFromBase(tempBase.string(), "hello.txt"); });

    // Test 2: Traversal blocked
    printResult("Test 2: traversal blocked", [&](){ return readFromBase(tempBase.string(), "../etc/passwd"); });

    // Test 3: Invalid chars
    printResult("Test 3: invalid chars blocked", [&](){ return readFromBase(tempBase.string(), "bad/name.txt"); });

    // Test 4: Missing file
    printResult("Test 4: non-existing file", [&](){ return readFromBase(tempBase.string(), "missing.txt"); });

    // Test 5: Directory instead of file
    printResult("Test 5: directory instead of file", [&](){ return readFromBase(tempBase.string(), "not_a_file"); });

    // Demo with fixed /safe (may fail)
    printResult("Demo: fixed /safe read (may fail)", [&](){ return readFromSafeDir("example.txt"); });

    return 0;
}