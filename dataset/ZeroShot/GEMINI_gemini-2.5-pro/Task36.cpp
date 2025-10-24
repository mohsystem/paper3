#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <cstdlib>

namespace fs = std::filesystem;

const fs::path SAFE_DIRECTORY = fs::absolute("safe_dir");

/**
 * @brief Securely reads a file and prints its contents to the console.
 * @param filename The name of the file to read (not the full path).
 */
void readFileAndPrint(const std::string& filename) {
    // Security Check 1: Basic validation. Filename should not contain path separators.
    if (filename.empty() || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos || filename == "..") {
        std::cerr << "Error: Invalid filename provided." << std::endl;
        return;
    }

    try {
        fs::path fullPath = SAFE_DIRECTORY / filename;

        // Security Check 2: Canonicalize path and verify it's within the safe directory.
        fs::path canonicalPath = fs::weakly_canonical(fullPath);
        std::string canonicalPathStr = canonicalPath.string();
        std::string safeDirStr = SAFE_DIRECTORY.string();
        if (canonicalPathStr.rfind(safeDirStr, 0) != 0) {
            std::cerr << "Error: Access denied. Path is outside the safe directory." << std::endl;
            return;
        }

        // Security Check 3: Ensure the path points to a regular file.
        if (!fs::is_regular_file(canonicalPath)) {
            std::cerr << "Error: Path does not point to a regular file or file does not exist." << std::endl;
            return;
        }

        std::cout << "--- Reading file: " << filename << " ---" << std::endl;
        std::ifstream fileStream(canonicalPath);
        if (!fileStream.is_open()) {
            std::cerr << "Error: Could not open file: " << filename << std::endl;
            return;
        }
        
        std::string line;
        while (std::getline(fileStream, line)) {
            std::cout << line << std::endl;
        }
        std::cout << "--- End of file: " << filename << " ---\n" << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
}

void setupTestEnvironment() {
    try {
        fs::create_directories(SAFE_DIRECTORY);
        std::ofstream(SAFE_DIRECTORY / "test1.txt") << "This is the first test file.";
        std::ofstream(SAFE_DIRECTORY / "test2.txt") << "This is the second test file.\nIt has multiple lines.";
        std::ofstream("insecure.txt") << "This file is outside the safe directory.";
        fs::create_directories(SAFE_DIRECTORY / "subdir");
        std::cout << "Setup complete. Safe directory is at: " << SAFE_DIRECTORY << std::endl;
        std::cout << "Running tests...\n" << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        exit(1);
    }
}

int main() {
    setupTestEnvironment();

    std::cout << "Test Case 1: Reading a valid file." << std::endl;
    readFileAndPrint("test1.txt");

    std::cout << "Test Case 2: Reading another valid file." << std::endl;
    readFileAndPrint("test2.txt");

    std::cout << "Test Case 3: Attempting to read a non-existent file." << std::endl;
    readFileAndPrint("nonexistent.txt");
    std::cout << std::endl;

    std::cout << "Test Case 4: Path traversal attempt." << std::endl;
    readFileAndPrint("../insecure.txt");
    std::cout << std::endl;

    std::cout << "Test Case 5: Attempting to read a directory." << std::endl;
    readFileAndPrint("subdir");
    std::cout << std::endl;

    return 0;
}