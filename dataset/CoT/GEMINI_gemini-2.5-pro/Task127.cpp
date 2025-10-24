#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <system_error>
#include <filesystem> // For C++17 filesystem operations

namespace fs = std::filesystem;

/**
 * Reads a file from sourcePath and writes its content to destPath.
 * Handles various file-related exceptions using modern C++ features.
 *
 * @param sourcePath The path of the file to read.
 * @param destPath The path of the file to write.
 * @return true on success, false on failure.
 */
bool copyFileWithHandling(const std::string& sourcePath, const std::string& destPath) {
    std::ifstream sourceFile;
    std::ofstream destFile;

    // Set exceptions to be thrown on failure
    sourceFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    destFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    try {
        sourceFile.open(sourcePath, std::ios::binary);
        destFile.open(destPath, std::ios::binary);

        // Efficiently copy file contents
        destFile << sourceFile.rdbuf();

        std::cout << "Successfully copied '" << sourcePath << "' to '" << destPath << "'." << std::endl;
        
        // RAII ensures files are closed when they go out of scope
        return true;
    } catch (const std::ios_base::failure& e) {
        // std::system_error is often nested inside and provides more info
        std::cerr << "Error: File operation failed. Details: " << e.what();
        if (e.code()) {
             std::cerr << " | System Code: " << e.code().value() << " (" << e.code().message() << ")";
        }
        std::cerr << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    
    return false;
}


int main() {
    // Setup for test cases
    fs::path tempDir = fs::temp_directory_path();
    fs::path sourceFilePath = tempDir / "source.txt";
    fs::path destFilePath = tempDir / "destination.txt";
    fs::path nonExistentSource = tempDir / "non_existent_file.txt";
    fs::path writeProtectedDir = tempDir / "protected_dir";

    // Create a dummy source file
    try {
        std::ofstream(sourceFilePath) << "This is a test file.";
        fs::create_directory(writeProtectedDir);
    } catch(const std::exception& e) {
        std::cerr << "Failed to set up test cases: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful file copy
    std::cout << "\n[Test Case 1: Successful Copy]" << std::endl;
    copyFileWithHandling(sourceFilePath.string(), destFilePath.string());

    // Test Case 2: Source file does not exist
    std::cout << "\n[Test Case 2: Source File Not Found]" << std::endl;
    copyFileWithHandling(nonExistentSource.string(), destFilePath.string());

    // Test Case 3: Reading a directory (simulates a read error)
    std::cout << "\n[Test Case 3: Read Error (source is a directory)]" << std::endl;
    copyFileWithHandling(writeProtectedDir.string(), destFilePath.string());

    // Test Case 4: Destination is not writable (it's a directory)
    std::cout << "\n[Test Case 4: Write Error (destination is a directory)]" << std::endl;
    copyFileWithHandling(sourceFilePath.string(), writeProtectedDir.string());
    
    // Test Case 5: Invalid path (empty)
    std::cout << "\n[Test Case 5: Invalid Argument (empty path)]" << std::endl;
    copyFileWithHandling("", destFilePath.string());


    // Cleanup
    try {
        fs::remove(sourceFilePath);
        fs::remove(destFilePath);
        fs::remove(writeProtectedDir);
    } catch(const std::exception& e) {
        std::cerr << "Cleanup failed: " << e.what() << std::endl;
    }

    return 0;
}