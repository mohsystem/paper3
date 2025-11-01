#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

// The base directory from which files can be read.
// Using a relative path for easier testing without root permissions.
const std::string SAFE_DIRECTORY = "safe";

/**
 * @brief Securely reads a file from a designated 'safe' directory.
 * 
 * It prevents directory traversal attacks by validating the filename and
 * ensuring the canonical path of the requested file is within the safe directory.
 * 
 * @param filename The name of the file to read. Must not contain path separators.
 * @return The content of the file as a std::string, or an error message.
 */
std::string readFileFromSafeDirectory(const std::string& filename) {
    // Validation 1: Filename should be simple, not a path.
    // .filename() returns the filename part of a path. If it's not equal to the
    // original string, it means the input contained directory separators.
    if (filename.empty() || fs::path(filename).filename().string() != filename) {
        return "Error: Invalid filename. It must not contain path separators.";
    }

    try {
        // Use absolute paths to prevent ambiguity. fs::canonical requires the path to exist.
        fs::path base_dir = fs::absolute(SAFE_DIRECTORY);
        fs::path requested_path = base_dir / filename;
        
        // Before calling canonical, check if the base directory itself exists.
        if (!fs::exists(base_dir) || !fs::is_directory(base_dir)) {
            return "Error: Safe directory does not exist.";
        }

        // Validation 2: Canonical path check.
        // fs::canonical resolves '..', '.', and symlinks. It throws on error (e.g., not found).
        fs::path canonical_path = fs::canonical(requested_path);

        // The core security check: Is the canonical path still within the base directory?
        // We do this by checking if the canonical path string starts with the base dir path string.
        auto base_str = base_dir.string();
        auto path_str = canonical_path.string();
        
        if (path_str.rfind(base_str, 0) != 0) {
            return "Error: Directory traversal attempt detected.";
        }

        // Ensure we are reading a regular file.
        if (!fs::is_regular_file(canonical_path)) {
            return "Error: Path does not point to a regular file.";
        }

        std::ifstream file(canonical_path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return "Error: Could not open file.";
        }
        
        // Read the whole file into a string
        std::string content(
            (std::istreambuf_iterator<char>(file)),
            std::istreambuf_iterator<char>()
        );
        file.close();
        return content;

    } catch (const fs::filesystem_error& e) {
        // This exception is often thrown by canonical() if the path doesn't exist.
        return "Error: File not found or filesystem error.";
    }
}

int main() {
    // --- Test Environment Setup ---
    try {
        if (fs::exists(SAFE_DIRECTORY)) {
            fs::remove_all(SAFE_DIRECTORY);
        }
        fs::create_directory(SAFE_DIRECTORY);
        std::ofstream test_file(fs::path(SAFE_DIRECTORY) / "document1.txt");
        test_file << "This is a safe document.";
        test_file.close();
        std::cout << "Test environment created." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        return 1;
    }
    
    // --- Test Cases ---
    std::vector<std::string> test_filenames = {
        "document1.txt",               // 1. Valid file
        "nonexistent.txt",             // 2. File not found
        "../main.cpp",                 // 3. Simple traversal attempt
        "document1.txt/../../main.cpp",// 4. Nested traversal attempt
        "/etc/passwd"                  // 5. Absolute path attempt
    };

    std::cout << "\n--- Running Test Cases ---" << std::endl;
    for (size_t i = 0; i < test_filenames.size(); ++i) {
        const std::string& filename = test_filenames[i];
        std::cout << "Test Case " << i + 1 << ": Requesting \"" << filename << "\"" << std::endl;
        std::string result = readFileFromSafeDirectory(filename);
        std::cout << "Result: " << result << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    // --- Test Environment Teardown ---
    try {
        fs::remove_all(SAFE_DIRECTORY);
        std::cout << "\nTest environment cleaned up." << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to clean up test environment: " << e.what() << std::endl;
    }

    return 0;
}