#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <stdexcept>

// For C++17 filesystem. Link with -lstdc++fs if needed on older GCC/Clang.
namespace fs = std::filesystem;

// Define the safe directory. In a real application, this should be an absolute path.
const std::string SAFE_DIRECTORY = "safe_dir";

/**
 * Reads a file from a predefined safe directory.
 * Prevents path traversal by canonicalizing paths and verifying that the
 * target file's path starts with the safe directory's path.
 *
 * @param filename The name of the file to read.
 * @return A string containing the file's content.
 * @throws std::runtime_error If the file cannot be read or if a security violation occurs.
 */
std::string readFileFromSafeDirectory(const std::string& filename) {
    try {
        // Get the canonical path of the safe directory.
        // canonical() resolves symlinks and '..'
        fs::path safeDirPath = fs::canonical(SAFE_DIRECTORY);

        // Construct the full path and canonicalize it.
        // weakly_canonical doesn't fail if the final component doesn't exist.
        fs::path targetFilePath = safeDirPath / filename;
        fs::path canonicalTargetPath = fs::weakly_canonical(targetFilePath);

        // Security Check: Ensure the canonical target path is within the canonical safe directory.
        // We do this by comparing the string representations of the paths.
        std::string safeDirStr = safeDirPath.string();
        std::string targetPathStr = canonicalTargetPath.string();

        if (targetPathStr.rfind(safeDirStr, 0) != 0) {
            throw std::runtime_error("Access Denied: Path traversal attempt detected for file: " + filename);
        }

        if (!fs::exists(canonicalTargetPath) || !fs::is_regular_file(canonicalTargetPath)) {
            throw std::runtime_error("File not found or is not a regular file: " + filename);
        }

        // Read the file content
        std::ifstream fileStream(canonicalTargetPath);
        if (!fileStream.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        return content;

    } catch (const fs::filesystem_error& e) {
        // This can catch errors from canonical() if the path is invalid
        throw std::runtime_error("File system error or access denied: " + std::string(e.what()));
    }
}

void setupEnvironment() {
    std::cout << "--- Setting up test environment ---" << std::endl;
    fs::path safeDir(SAFE_DIRECTORY);
    
    // Clean up previous runs
    if (fs::exists(safeDir)) {
        fs::remove_all(safeDir);
    }
    
    fs::create_directories(safeDir / "sub");

    std::ofstream(safeDir / "good_file.txt") << "This is a safe file.";
    std::ofstream(safeDir / "sub" / "nested_file.txt") << "This is a nested safe file.";
    std::ofstream("evil.txt") << "This file is outside the safe directory.";
    
    std::cout << "Setup complete." << std::endl << std::endl;
}

void cleanupEnvironment() {
    fs::remove("evil.txt");
    fs::remove_all(SAFE_DIRECTORY);
}

int main() {
    setupEnvironment();
    
    std::vector<std::string> testCases = {
        "good_file.txt",            // 1. Should succeed
        "sub/nested_file.txt",      // 2. Should succeed (subdirectory)
        "non_existent.txt",         // 3. Should fail (not found)
        "../evil.txt",              // 4. Should fail (path traversal)
        "sub/../../evil.txt"        // 5. Should fail (complex path traversal)
    };
    
    std::cout << "--- Running Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Attempting to read: '" << testCase << "'" << std::endl;
        try {
            std::string content = readFileFromSafeDirectory(testCase);
            std::cout << "  [SUCCESS] Content: \"" << content << "\"" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "  [FAILURE] Exception: " << e.what() << std::endl;
        }
        std::cout << "---------------------------------" << std::endl;
    }
    
    cleanupEnvironment();
    return 0;
}