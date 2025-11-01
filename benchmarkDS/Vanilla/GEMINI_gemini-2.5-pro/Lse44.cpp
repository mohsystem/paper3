#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem> // Requires C++17

namespace fs = std::filesystem;

/**
 * Reads a file from the 'safe' directory, preventing path traversal.
 * @param filename The name of the file to read.
 * @return The content of the file as a string, or an error message.
 */
std::string readFileFromSafeDirectory(const std::string& filename) {
    // Basic check for traversal sequences or absolute paths
    if (filename.empty() || filename.find("..") != std::string::npos || fs::path(filename).is_absolute()) {
        return "Error: Invalid filename (contains traversal characters or is absolute).";
    }

    // Use a relative path for the safe directory for testing purposes.
    fs::path safe_dir_path = fs::absolute("safe");
    fs::path requested_path = safe_dir_path / filename;
    
    // weakly_canonical resolves '..' and symlinks without requiring the path to exist
    fs::path canonical_path = fs::weakly_canonical(requested_path);
    
    // Security check: ensure the canonical path starts with the safe directory path
    std::string safe_dir_str = safe_dir_path.string();
    std::string canonical_str = canonical_path.string();

    if (canonical_str.rfind(safe_dir_str, 0) != 0) {
        return "Error: Directory traversal attempt detected.";
    }

    if (!fs::exists(canonical_path) || !fs::is_regular_file(canonical_path)) {
        return "Error: File not found or is not a regular file.";
    }

    std::ifstream file_stream(canonical_path);
    if (!file_stream.is_open()) {
        return "Error: Could not open file.";
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    return buffer.str();
}

void setup_environment() {
    try {
        if (fs::exists("safe")) {
            fs::remove_all("safe");
        }
        fs::create_directory("safe");
        std::ofstream("safe/test1.txt") << "This is test file 1.";
        std::ofstream("safe/another_file.log") << "Log entry.";
        std::cout << "Test environment set up." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Setup failed: " << e.what() << std::endl;
    }
}

void cleanup_environment() {
    try {
        if (fs::exists("safe")) {
            fs::remove_all("safe");
            std::cout << "\nTest environment cleaned up." << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Cleanup failed: " << e.what() << std::endl;
    }
}

int main() {
    setup_environment();

    std::vector<std::string> test_files = {
        "test1.txt",              // 1. Valid file
        "another_file.log",       // 2. Another valid file
        "nonexistent.txt",        // 3. Non-existent file
        "../secret.txt",          // 4. Directory traversal attempt
        "../../etc/passwd"        // 5. More complex traversal attempt
    };
    
    std::cout << "\n--- Running Test Cases ---" << std::endl;
    int i = 1;
    for (const auto& filename : test_files) {
        std::cout << "Test Case " << i++ << ": Requesting '" << filename << "'" << std::endl;
        std::string content = readFileFromSafeDirectory(filename);
        std::cout << "Result: " << content << std::endl;
        std::cout << "----------------------------" << std::endl;
    }

    cleanup_environment();
    return 0;
}