#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <optional>
#include <sstream>

// Requires C++17 or later for std::filesystem
namespace fs = std::filesystem;

const std::string SAFE_DIRECTORY_NAME = "safe_dir";

/**
 * @brief Securely reads a file from a predefined safe directory.
 * 
 * It prevents path traversal attacks by canonicalizing paths and verifying
 * the requested file is strictly within the safe directory.
 * 
 * @param filename The name of the file to read.
 * @return An std::optional containing the file content as a string if successful,
 *         otherwise std::nullopt.
 */
std::optional<std::string> readFileFromSafeDirectory(const std::string& filename) {
    if (filename.empty() || filename.find('\0') != std::string::npos) {
        std::cerr << "Error: Filename is invalid (empty or contains null byte)." << std::endl;
        return std::nullopt;
    }

    std::error_code ec;

    // Get the canonical path of the base directory. This must exist.
    const auto safe_dir_canon = fs::canonical(SAFE_DIRECTORY_NAME, ec);
    if (ec) {
        std::cerr << "Error: Safe directory '" << SAFE_DIRECTORY_NAME << "' is not accessible." << std::endl;
        return std::nullopt;
    }

    // Construct the full path and normalize it lexically.
    // This simplifies "foo/./bar" to "foo/bar" but does not resolve ".." yet.
    fs::path target_path = (safe_dir_canon / filename).lexically_normal();

    // Use weakly_canonical to resolve ".." and symlinks without requiring the final component to exist.
    const auto target_canon = fs::weakly_canonical(target_path, ec);
    if (ec) {
        std::cerr << "Error: Could not resolve path for '" << filename << "'." << std::endl;
        return std::nullopt;
    }

    // The crucial security check: is the resolved target path still within the resolved safe directory?
    // We check if safe_dir_canon is a prefix of target_canon.
    // A simple string comparison is the most portable and effective way.
    if (target_canon.string().rfind(safe_dir_canon.string(), 0) != 0) {
        std::cerr << "Security Error: Path traversal attempt detected for file: " << filename << std::endl;
        return std::nullopt;
    }

    // Final check to ensure we're reading a regular file that exists.
    if (!fs::is_regular_file(target_canon, ec) || ec) {
        std::cerr << "Error: Path does not point to a regular file: " << filename << std::endl;
        return std::nullopt;
    }

    std::ifstream file_stream(target_canon, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        std::cerr << "Error: Could not open file: " << filename << std::endl;
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    return buffer.str();
}

void setupTestEnvironment() {
    std::cout << "--- Setting up test environment ---" << std::endl;
    try {
        fs::create_directories(SAFE_DIRECTORY_NAME + "/subdir");

        std::ofstream(fs::path(SAFE_DIRECTORY_NAME) / "test1.txt") << "This is a safe file.";
        std::ofstream(fs::path(SAFE_DIRECTORY_NAME) / "subdir" / "test2.txt") << "This is another safe file in a subdirectory.";
        std::ofstream("secret.txt") << "This is a secret file.";
        
        std::cout << "Setup complete." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
}

void cleanupTestEnvironment() {
    std::cout << "\n--- Cleaning up test environment ---" << std::endl;
    std::error_code ec;
    fs::remove_all(SAFE_DIRECTORY_NAME, ec);
    fs::remove("secret.txt", ec);
    std::cout << "Cleanup complete." << std::endl;
}

class Lse45 {
public:
    static int main() {
        setupTestEnvironment();
        std::cout << "\n--- Running Test Cases ---" << std::endl;

        // Test Case 1: Read a valid file
        std::cout << "\n[Test 1] Reading a valid file 'test1.txt'" << std::endl;
        if (auto content = readFileFromSafeDirectory("test1.txt")) {
            std::cout << "SUCCESS. Content: " << *content << std::endl;
        } else {
            std::cout << "FAILED." << std::endl;
        }

        // Test Case 2: Read a non-existent file
        std::cout << "\n[Test 2] Reading a non-existent file 'nonexistent.txt'" << std::endl;
        if (auto content = readFileFromSafeDirectory("nonexistent.txt")) {
            std::cout << "SUCCESS. Content: " << *content << std::endl;
        } else {
            std::cout << "FAILED. (Expected)" << std::endl;
        }

        // Test Case 3: Path traversal attempt
        std::cout << "\n[Test 3] Path traversal attempt '../secret.txt'" << std::endl;
        if (auto content = readFileFromSafeDirectory("../secret.txt")) {
            std::cout << "SUCCESS. Content: " << *content << std::endl;
        } else {
            std::cout << "FAILED. (Expected Security Block)" << std::endl;
        }

        // Test Case 4: Path traversal from a subdirectory
        std::cout << "\n[Test 4] Path traversal from subdirectory 'subdir/../../secret.txt'" << std::endl;
        if (auto content = readFileFromSafeDirectory("subdir/../../secret.txt")) {
            std::cout << "SUCCESS. Content: " << *content << std::endl;
        } else {
            std::cout << "FAILED. (Expected Security Block)" << std::endl;
        }

        // Test Case 5: Reading a valid file in a subdirectory
        std::cout << "\n[Test 5] Reading a valid file in a subdirectory 'subdir/test2.txt'" << std::endl;
        if (auto content = readFileFromSafeDirectory("subdir/test2.txt")) {
            std::cout << "SUCCESS. Content: " << *content << std::endl;
        } else {
            std::cout << "FAILED." << std::endl;
        }
        
        cleanupTestEnvironment();
        return 0;
    }
};

int main() {
    return Lse45::main();
}