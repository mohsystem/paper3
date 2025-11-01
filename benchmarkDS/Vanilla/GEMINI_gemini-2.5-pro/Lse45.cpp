#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <sstream>

// For C++17 filesystem. Link with -lstdc++fs if needed.
namespace fs = std::filesystem;

const std::string SAFE_DIR = "safe_dir";

/**
 * @brief Reads a file from a safe directory, preventing directory traversal.
 * 
 * @param filename The name of the file to be read.
 * @return std::string The content of the file or an error message.
 */
std::string readFileFromSafeDir(const std::string& filename) {
    if (filename.empty() || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        return "Error: Invalid characters in filename.";
    }

    try {
        fs::path safeDirPath = fs::absolute(SAFE_DIR);
        fs::path requestedPath = safeDirPath / filename;

        // Normalize the path to resolve any "." or symbolic links
        // weakly_canonical handles non-existent paths gracefully
        requestedPath = fs::weakly_canonical(requestedPath);

        // Security Check: Ensure the canonical path is within the safe directory.
        auto [root, rel] = std::mismatch(safeDirPath.begin(), safeDirPath.end(), requestedPath.begin());
        if (root != safeDirPath.end()) {
             return "Error: Directory traversal attempt detected.";
        }
        
        if (!fs::exists(requestedPath)) {
            return "Error: File not found: " + filename;
        }

        if (!fs::is_regular_file(requestedPath)) {
            return "Error: Requested path is not a regular file.";
        }

        std::ifstream fileStream(requestedPath);
        if (!fileStream.is_open()) {
            return "Error: Could not open file stream.";
        }

        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();

    } catch (const fs::filesystem_error& e) {
        return "Error: Filesystem error: " + std::string(e.what());
    }
}

/**
 * @brief Sets up the test environment by creating a safe directory and files.
 */
void setupTestEnvironment() {
    try {
        if (fs::exists(SAFE_DIR)) {
            fs::remove_all(SAFE_DIR);
        }
        fs::create_directory(SAFE_DIR);

        std::ofstream file1(fs::path(SAFE_DIR) / "file1.txt");
        if (file1.is_open()) {
            file1 << "This is file 1 from C++.";
            file1.close();
        }

        std::ofstream file2(fs::path(SAFE_DIR) / "file2.txt");
        if (file2.is_open()) {
            file2 << "Content of the second file.";
            file2.close();
        }
        std::cout << "Test environment created in './" << SAFE_DIR << "'" << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
}


int main() {
    setupTestEnvironment();
    std::cout << "\n--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Read a valid file
    std::cout << "1. Reading 'file1.txt':" << std::endl;
    std::cout << "   Result: " << readFileFromSafeDir("file1.txt") << std::endl << std::endl;

    // Test Case 2: Read another valid file
    std::cout << "2. Reading 'file2.txt':" << std::endl;
    std::cout << "   Result: " << readFileFromSafeDir("file2.txt") << std::endl << std::endl;
    
    // Test Case 3: Attempt to read a non-existent file
    std::cout << "3. Reading 'nonexistent.txt':" << std::endl;
    std::cout << "   Result: " << readFileFromSafeDir("nonexistent.txt") << std::endl << std::endl;

    // Test Case 4: Attempt directory traversal (simple)
    std::cout << "4. Attempting to read '../some_file.txt':" << std::endl;
    std::cout << "   Result: " << readFileFromSafeDir("../some_file.txt") << std::endl << std::endl;
    
    // Test Case 5: Attempt directory traversal with null byte (common trick)
    std::cout << "5. Attempting to read '../some_file.txt\\0other.txt':" << std::endl;
    std::cout << "   Result: " << readFileFromSafeDir("../some_file.txt\0other.txt") << std::endl << std::endl;

    return 0;
}