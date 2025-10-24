#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Copies a source file to a destination file, handling exceptions.
 * @param sourcePath Path to the source file.
 * @param destPath Path to the destination file.
 */
void copyFile(const std::string& sourcePath, const std::string& destPath) {
    std::ifstream inFile;
    std::ofstream outFile;

    try {
        // Open the source file for reading
        inFile.open(sourcePath);
        if (!inFile.is_open()) {
            // Check if it's a directory
            if (fs::is_directory(sourcePath)) {
                throw std::runtime_error("Source path is a directory, not a file.");
            }
            throw std::runtime_error("Could not open source file for reading.");
        }

        // Ensure the destination directory exists
        fs::path destFsPath(destPath);
        if (destFsPath.has_parent_path()) {
            fs::create_directories(destFsPath.parent_path());
        }

        // Open the destination file for writing
        outFile.open(destPath);
        if (!outFile.is_open()) {
            throw std::runtime_error("Could not open destination file for writing.");
        }

        // Copy content
        outFile << inFile.rdbuf();

        std::cout << "Success: File copied from " << sourcePath << " to " << destPath << std::endl;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Filesystem operation failed: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    // fstream destructors will close files automatically (RAII)
}

int main() {
    std::string testDir = "cpp_test_files";
    std::string validInputFile = testDir + "/input.txt";
    std::string validOutputFile = testDir + "/output.txt";
    std::string readOnlyDir = testDir + "/read_only_dir";

    // Setup test environment
    fs::remove_all(testDir);
    fs::create_directories(testDir);
    std::ofstream(validInputFile) << "This is a test file for C++.\n" << "It has multiple lines.\n";
    fs::create_directories(readOnlyDir);
    
    // Set directory to read-only. This might fail on some systems/setups.
    try {
        fs::permissions(readOnlyDir, fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read, fs::perm_options::replace);
    } catch(const fs::filesystem_error& e) {
        std::cerr << "Warning: Could not set directory permissions for '" << readOnlyDir << "'. Test 4 might not behave as expected. " << e.what() << std::endl;
    }

    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful file copy
    std::cout << "\n[Test Case 1: Successful Copy]" << std::endl;
    copyFile(validInputFile, validOutputFile);

    // Test Case 2: Input file not found
    std::cout << "\n[Test Case 2: Input File Not Found]" << std::endl;
    copyFile(testDir + "/non_existent.txt", validOutputFile);

    // Test Case 3: Read error (trying to read a directory)
    std::cout << "\n[Test Case 3: Read Error (Reading a directory)]" << std::endl;
    copyFile(testDir, validOutputFile);

    // Test Case 4: Write error (permission denied)
    std::cout << "\n[Test Case 4: Write Error (Permission Denied)]" << std::endl;
    copyFile(validInputFile, readOnlyDir + "/output.txt");

    // Test Case 5: Write error (invalid path where part of the path is a file)
    std::cout << "\n[Test Case 5: Write Error (Invalid Path)]" << std::endl;
    copyFile(validInputFile, validInputFile + "/output.txt");

    // Cleanup
    fs::remove_all(testDir);
    std::cout << "\n--- C++ Tests Complete ---" << std::endl;

    return 0;
}