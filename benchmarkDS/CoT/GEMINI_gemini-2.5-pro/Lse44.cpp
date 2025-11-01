#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

const std::string SAFE_DIRECTORY = "/safe/";

/**
 * Reads a file from the predefined SAFE_DIRECTORY, preventing path traversal.
 *
 * @param filename The name of the file to read.
 * @return A string containing the file content or an error message.
 */
std::string readFile(const std::string& filename) {
    // Security: Basic check for traversal sequences
    if (filename.empty() || filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        return "Error: Invalid filename provided.";
    }

    try {
        fs::path safeDirPath = fs::absolute(SAFE_DIRECTORY);
        fs::path requestedPath = safeDirPath / filename;

        // Security: Canonicalize the path to resolve symlinks, '..', etc.
        // weakly_canonical doesn't throw an error if the path doesn't exist.
        fs::path canonicalPath = fs::weakly_canonical(requestedPath);

        // Security Check: Verify the canonical path starts with the safe directory's canonical path.
        std::string canonPathStr = canonicalPath.string();
        std::string safeDirStr = safeDirPath.string();

        if (canonPathStr.rfind(safeDirStr, 0) != 0) {
            return "Error: Directory traversal attempt detected.";
        }
        
        if (fs::is_regular_file(canonicalPath)) {
            std::ifstream file(canonicalPath);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            } else {
                return "Error: Could not open the file.";
            }
        } else {
            return "Error: File is not readable or does not exist.";
        }

    } catch (const fs::filesystem_error& e) {
        return "Error: Filesystem error occurred. " + std::string(e.what());
    } catch (const std::exception& e) {
        return "Error: An unexpected error occurred. " + std::string(e.what());
    }
}


int main() {
    // Setup test environment
    try {
        fs::create_directories(SAFE_DIRECTORY);
        std::ofstream(SAFE_DIRECTORY + "test1.txt") << "This is a safe file.";
        std::ofstream(SAFE_DIRECTORY + "test2.log") << "Another safe log file.";
        std::ofstream("unsafe.txt") << "This is an unsafe file.";
    } catch(const std::exception& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid file
    std::cout << "1. Reading 'test1.txt':" << std::endl;
    std::cout << readFile("test1.txt") << std::endl << std::endl;

    // Test Case 2: Another valid file
    std::cout << "2. Reading 'test2.log':" << std::endl;
    std::cout << readFile("test2.log") << std::endl << std::endl;

    // Test Case 3: Non-existent file
    std::cout << "3. Reading 'nonexistent.txt':" << std::endl;
    std::cout << readFile("nonexistent.txt") << std::endl << std::endl;

    // Test Case 4: Path traversal attempt
    std::cout << "4. Reading '../unsafe.txt':" << std::endl;
    std::cout << readFile("../unsafe.txt") << std::endl << std::endl;

    // Test Case 5: Absolute path attempt
    std::cout << "5. Reading '/etc/passwd':" << std::endl;
    std::cout << readFile("/etc/passwd") << std::endl << std::endl;

    return 0;
}