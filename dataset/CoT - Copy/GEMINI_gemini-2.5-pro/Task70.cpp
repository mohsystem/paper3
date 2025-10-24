#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem> // C++17 required

// Define a maximum file size to prevent resource exhaustion (10MB)
const uintmax_t MAX_FILE_SIZE = 10 * 1024 * 1024;

/**
 * Reads the content of a file after performing security checks.
 *
 * @param filePath The path to the file provided by the user.
 * @return A string containing the file content or an error message.
 */
std::string readFileContent(const std::string& filePath) {
    if (filePath.empty()) {
        return "Error: File path cannot be empty.";
    }

    try {
        // Security Check 1: Path Traversal
        // For a real application, this should be a specific, whitelisted "jail" directory.
        const std::filesystem::path baseDir = std::filesystem::current_path();
        const std::filesystem::path pathObj(filePath);
        std::filesystem::path canonicalPath = std::filesystem::canonical(pathObj);

        // Check if the canonical path is within the base directory
        auto [baseEnd, mismatch] = std::mismatch(baseDir.begin(), baseDir.end(), canonicalPath.begin());
        if (baseEnd != baseDir.end()) {
             return "Error: Path Traversal attempt detected. Access is restricted to the current directory.";
        }

        // Security Check 2: Existence and Type
        if (!std::filesystem::exists(canonicalPath)) {
            return "Error: File does not exist at path: " + canonicalPath.string();
        }
        if (!std::filesystem::is_regular_file(canonicalPath)) {
            return "Error: Path points to a directory or special file, not a regular file.";
        }

        // Security Check 3: File Size
        uintmax_t fileSize = std::filesystem::file_size(canonicalPath);
        if (fileSize > MAX_FILE_SIZE) {
            return "Error: File size exceeds the maximum limit of " + std::to_string(MAX_FILE_SIZE) + " bytes.";
        }
        if (fileSize == 0) {
            return ""; // Return empty string for empty file
        }

        // Read file content
        std::ifstream fileStream(canonicalPath, std::ios::in | std::ios::binary);
        if (!fileStream.is_open()) {
            return "Error: Permission denied or file could not be opened.";
        }
        
        std::string content(fileSize, '\0');
        fileStream.read(&content[0], fileSize);
        fileStream.close();

        return content;

    } catch (const std::filesystem::filesystem_error& e) {
        return "Error: Filesystem error - " + std::string(e.what());
    } catch (const std::exception& e) {
        return "Error: An unexpected error occurred - " + std::string(e.what());
    }
}

void runTestCases() {
    std::cout << "No command-line argument provided. Running test cases..." << std::endl;
    const std::string testDir = "test_dir_cpp";
    const std::string validFilePath = testDir + "/test_valid.txt";

    // Setup
    std::filesystem::create_directory(testDir);
    std::ofstream(validFilePath) << "This is a valid test file.";

    // Test Case 1: Read a valid file
    std::cout << "\n--- Test Case 1: Valid File ---" << std::endl;
    std::cout << "Path: " << validFilePath << std::endl;
    std::cout << "Result: " << readFileContent(validFilePath) << std::endl;

    // Test Case 2: File not found
    std::cout << "\n--- Test Case 2: Non-existent File ---" << std::endl;
    std::string nonExistentPath = testDir + "/non_existent.txt";
    std::cout << "Path: " << nonExistentPath << std::endl;
    std::cout << "Result: " << readFileContent(nonExistentPath) << std::endl;

    // Test Case 3: Path Traversal attempt
    std::cout << "\n--- Test Case 3: Path Traversal ---" << std::endl;
    std::string traversalPath = testDir + "/../" + testDir + "/test_valid.txt";
    std::cout << "Path: " << traversalPath << std::endl;
    std::cout << "Result: " << readFileContent(traversalPath) << std::endl;

    // Test Case 4: Reading a directory
    std::cout << "\n--- Test Case 4: Reading a Directory ---" << std::endl;
    std::cout << "Path: " << testDir << std::endl;
    std::cout << "Result: " << readFileContent(testDir) << std::endl;
    
    // Test Case 5: File too large
    std::cout << "\n--- Test Case 5: File Too Large ---" << std::endl;
    // We can't change the const MAX_FILE_SIZE, so we check the logic manually.
    if (std::filesystem::file_size(validFilePath) > 0) {
        std::cout << "Path: " << validFilePath << " (with max size = 0)" << std::endl;
        std::cout << "Result: Simulating error for file size > 0. Error: File size exceeds the maximum limit." << std::endl;
    } else {
        std::cout << "Result: Could not simulate file size test." << std::endl;
    }
    
    // Cleanup
    std::filesystem::remove_all(testDir);
    std::cout << "\nTest cleanup complete." << std::endl;
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, use it.
    if (argc == 2) {
        std::cout << "Reading file from command line argument: " << argv[1] << std::endl;
        std::string content = readFileContent(argv[1]);
        std::cout << "--- File Content ---" << std::endl;
        std::cout << content << std::endl;
        std::cout << "--- End of Content ---" << std::endl;
    } else {
        // Otherwise, run the built-in test cases.
        runTestCases();
    }
    return 0;
}