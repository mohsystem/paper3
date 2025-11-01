#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>

std::string read_safe_file(const std::string& requestedPath) {
    namespace fs = std::filesystem;
    fs::path base = fs::canonical(fs::path("/safe"));
    fs::path candidate = fs::path("/safe") / fs::path(requestedPath);
    fs::path norm = fs::weakly_canonical(candidate);

    std::string baseStr = base.generic_string();
    std::string normStr = norm.generic_string();
    if (normStr.rfind(baseStr, 0) != 0) {
        throw std::invalid_argument("Access outside of /safe is not allowed");
    }
    if (!fs::exists(norm) || !fs::is_regular_file(norm)) {
        throw std::runtime_error("File does not exist or is not a regular file");
    }

    std::ifstream in(norm, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file");
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

int main() {
    const char* tests[5] = {
        "example.txt",
        "subdir/data.txt",
        "../etc/passwd",
        "/etc/passwd",
        "nonexistent.txt"
    };
    for (const char* t : tests) {
        try {
            std::string content = read_safe_file(t);
            std::cout << "[" << t << "] CONTENT:\n" << content << "\n";
        } catch (const std::exception& e) {
            std::cout << "[" << t << "] ERROR: " << e.what() << "\n";
        }
        std::cout << "----\n";
    }
    return 0;
}