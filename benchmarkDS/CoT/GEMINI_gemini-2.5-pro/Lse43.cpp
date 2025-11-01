#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem> // C++17 required

// For convenience
namespace fs = std::filesystem;

// Define the base directory. For testing, we use a temporary directory.
// In a real application, this would be a fixed path like "/safe/".
const fs::path SAFE_DIR = "test_root/safe/";

/**
 * Reads a file from a designated "safe" directory, preventing path traversal.
 *
 * @param filename The name of the file to read. Must not contain path separators.
 * @return The content of the file as a string, or an error message.
 */
std::string readFileFromSafeDirectory(const std::string& filename) {
    // 1. Initial validation: Filename should not contain path separators.
    if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        return "Error: Invalid characters in filename. Path traversal attempt rejected.";
    }

    fs::path targetPath = SAFE_DIR / filename;

    try {
        // 2. Canonical path validation: This resolves '..', symlinks, etc.
        // It throws an exception if the path does not exist, which is a good
        // first check.
        fs::path canonicalSafePath = fs::canonical(SAFE_DIR);
        fs::path canonicalTargetPath = fs::canonical(targetPath);

        // Convert to strings to check if the target path starts with the safe path.
        std::string safeStr = canonicalSafePath.string();
        std::string targetStr = canonicalTargetPath.string();
        
        // Ensure the target path is genuinely inside the safe directory.
        // The rfind check is a robust way to implement starts_with.
        if (targetStr.rfind(safeStr, 0) != 0) {
            return "Error: Path traversal attempt detected.";
        }

        // 3. Read the file if validation passes.
        std::ifstream fileStream(canonicalTargetPath);
        if (!fileStream.is_open()) {
            return "Error: Could not open file for reading.";
        }
        
        std::string content(
            (std::istreambuf_iterator<char>(fileStream)),
            (std::istreambuf_iterator<char>())
        );
        return content;

    } catch (const fs::filesystem_error& e) {
        // This catches errors from fs::canonical (e.g., file not found)
        // and other filesystem issues.
        return std::string("Error: Could not read file. ") + e.what();
    }
}

// RAII class for test environment setup and cleanup
struct TestEnvironment {
    TestEnvironment() {
        std::cout << "--- Setting up test environment ---" << std::endl;
        if (fs::exists("test_root")) {
            fs::remove_all("test_root");
        }
        fs::create_directories(SAFE_DIR);
        
        std::ofstream(SAFE_DIR / "good.txt") << "This is a safe file.";
        std::ofstream(SAFE_DIR / "another.txt") << "This is another safe file.";
        std::ofstream("test_root/outside.txt") << "This file is outside the safe directory.";
        std::cout << "Setup complete." << std::endl;
        std::cout << "-------------------------------------\n" << std::endl;
    }

    ~TestEnvironment() {
        if (fs::exists("test_root")) {
            fs::remove_all("test_root");
        }
        // std::cout << "Test environment cleaned up." << std::endl;
    }
};

int main() {
    TestEnvironment env; // Setup and cleanup are automatic

    std::vector<std::string> testFilenames = {
        "good.txt",                      // Test Case 1: Valid file
        "another.txt",                   // Test Case 2: Another valid file
        "nonexistent.txt",               // Test Case 3: File does not exist
        "../outside.txt",                // Test Case 4: Path traversal attempt
        "good.txt/../../outside.txt"     // Test Case 5: Complex path traversal attempt
    };

    for (int i = 0; i < testFilenames.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ": Attempting to read '" << testFilenames[i] << "'" << std::endl;
        std::string result = readFileFromSafeDirectory(testFilenames[i]);
        std::cout << "Result: " << result << "\n" << std::endl;
    }
    
    return 0;
}