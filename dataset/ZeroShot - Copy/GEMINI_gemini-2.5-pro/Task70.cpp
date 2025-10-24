#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <system_error>
#include <vector>

// C++17 is required for std::filesystem

namespace fs = std::filesystem;

/**
 * Securely reads the content of a file specified by a path.
 * The path is validated to prevent directory traversal attacks. The file must
 * exist within the current working directory.
 *
 * @param userPath The path to the file provided by the user.
 * @return The content of the file as a string, or an error message if the operation fails.
 */
std::string readFileContent(const std::string& userPath) {
    if (userPath.empty()) {
        return "Error: File path cannot be empty.";
    }

    try {
        // Define the allowed base directory (current working directory)
        const fs::path baseDir = fs::current_path();
        
        // Construct the full path and get its canonical form.
        // fs::canonical resolves ".." and symbolic links, and throws an error if path doesn't exist.
        // This is a crucial security step.
        fs::path requestedPath = baseDir / fs::path(userPath).relative_path();
        fs::path canonicalPath = fs::canonical(requestedPath);
        fs::path canonicalBase = fs::canonical(baseDir);

        // Security Check 1: Ensure the requested path is within the base directory.
        // We do this by checking if the canonical path string starts with the base path string.
        std::string canonicalPathStr = canonicalPath.string();
        std::string canonicalBaseStr = canonicalBase.string();

        if (canonicalPathStr.rfind(canonicalBaseStr, 0) != 0) {
            return "Error: Directory traversal attempt detected. Access denied.";
        }

        // Security Check 2: Ensure the path points to a regular file.
        if (!fs::is_regular_file(canonicalPath)) {
            return "Error: Path does not point to a regular file.";
        }
        
        // Security Check 3: Check file size to prevent DoS (e.g., 10MB)
        uintmax_t fileSize = fs::file_size(canonicalPath);
        uintmax_t maxSize = 10 * 1024 * 1024; // 10 MB
        if (fileSize > maxSize) {
            return "Error: File is too large.";
        }

        // Read the file content
        std::ifstream fileStream(canonicalPath, std::ios::in | std::ios::binary);
        if (!fileStream) {
            return "Error: Could not open file for reading.";
        }

        std::stringstream buffer;
        buffer << fileStream.rdbuf();
        return buffer.str();

    } catch (const fs::filesystem_error& e) {
        // This exception is thrown by fs::canonical if the path does not exist
        // or by other filesystem operations on error.
        return std::string("Error: Filesystem error - ") + e.what();
    } catch (const std::exception& e) {
        return std::string("Error: An unexpected error occurred - ") + e.what();
    }
}


void run_tests() {
    std::cout << "Running integrated test cases..." << std::endl;
    fs::path testSubDir("test_dir");
    fs::path safeFile("safe.txt");
    fs::path nestedFile = testSubDir / "nested.txt";
    
    // Setup
    try {
        fs::create_directory(testSubDir);
        std::ofstream(safeFile) << "This is a safe file.";
        std::ofstream(nestedFile) << "This is a nested file.";
    } catch(const std::exception& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
        return;
    }

    // --- Test Cases ---
    std::cout << "\n--- Test Case 1: Valid file in CWD ---" << std::endl;
    std::cout << "Input: " << safeFile.string() << std::endl;
    std::cout << "Output: \n" << readFileContent(safeFile.string()) << std::endl;

    std::cout << "\n--- Test Case 2: Valid nested file ---" << std::endl;
    std::cout << "Input: " << nestedFile.string() << std::endl;
    std::cout << "Output: \n" << readFileContent(nestedFile.string()) << std::endl;

    std::cout << "\n--- Test Case 3: Path Traversal attempt ---" << std::endl;
    std::string traversalPath = "../safe.txt";
    std::cout << "Input: " << traversalPath << std::endl;
    std::cout << "Output: \n" << readFileContent(traversalPath) << std::endl;

    std::cout << "\n--- Test Case 4: Non-existent file ---" << std::endl;
    std::string nonExistentFile = "nonexistent.txt";
    std::cout << "Input: " << nonExistentFile << std::endl;
    std::cout << "Output: \n" << readFileContent(nonExistentFile) << std::endl;

    std::cout << "\n--- Test Case 5: Attempt to read a directory ---" << std::endl;
    std::cout << "Input: " << testSubDir.string() << std::endl;
    std::cout << "Output: \n" << readFileContent(testSubDir.string()) << std::endl;

    // Cleanup
    try {
        fs::remove_all(testSubDir);
        fs::remove(safeFile);
    } catch(const std::exception& e) {
        std::cerr << "Failed to clean up test environment: " << e.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        std::cout << "Reading file from command line argument: " << argv[1] << std::endl;
        std::string content = readFileContent(argv[1]);
        std::cout << "--- File Content ---" << std::endl;
        std::cout << content << std::endl;
        std::cout << "--- End of Content ---" << std::endl;
    } else {
        run_tests();
    }

    return 0;
}