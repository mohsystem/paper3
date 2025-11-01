#include <iostream>
#include <string>
#include <cstdlib> // For system()
#include <filesystem> // C++17
#include <fstream>

// C++17 filesystem alias
namespace fs = std::filesystem;

/**
 * Extracts a zip file to a specified directory using the system's 'unzip' command.
 * Note: This function requires the 'unzip' command-line tool to be installed and in the system's PATH.
 *
 * @param zipFilePath Path to the .zip file.
 * @param destDirectory Path to the destination directory.
 * @return True on success, false on failure.
 */
bool extractZip(const std::string& zipFilePath, const std::string& destDirectory) {
    if (!fs::exists(zipFilePath)) {
        std::cerr << "Error: ZIP file not found at '" << zipFilePath << "'" << std::endl;
        return false;
    }

    try {
        // Create destination directory if it doesn't exist
        fs::create_directories(destDirectory);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating directory " << destDirectory << ": " << e.what() << std::endl;
        return false;
    }

    // Construct the command.
    // -o: overwrite files without prompting. -q: quiet mode.
    std::string command = "unzip -o -q \"" + zipFilePath + "\" -d \"" + destDirectory + "\" > /dev/null 2>&1";

    // Execute the command
    int result = system(command.c_str());

    if (result == 0) {
        // std::cout << "Successfully extracted '" << zipFilePath << "' to '" << destDirectory << "'" << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to extract '" << zipFilePath << "'. Make sure 'unzip' is installed." << std::endl;
        return false;
    }
}

// Helper to check file content
bool checkFileContent(const fs::path& path, const std::string& expectedContent) {
    if (!fs::exists(path)) return false;
    std::ifstream file(path);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    // Remove potential trailing newline from file read
    if (!content.empty() && content.back() == '\n') {
        content.pop_back();
    }
    return content == expectedContent;
}

int main() {
    // Note: This implementation requires 'zip' and 'unzip' command-line tools and a C++17 compiler.
    // The system calls are tailored for Linux/macOS.
    std::cout << "Running C++ Lse50 tests..." << std::endl;
    const std::string destDir = "/tmp/unpack";
    const fs::path destPath(destDir);
    
    // Setup and Cleanup helpers
    auto setup = [](const std::string& zip_cmd) { system(zip_cmd.c_str()); };
    auto cleanup = [&](const std::string& zipFile) { 
        if (fs::exists(zipFile)) fs::remove(zipFile);
        if (fs::exists(destPath)) fs::remove_all(destPath);
        if (fs::exists("test_setup")) fs::remove_all("test_setup");
    };

    // Test Case 1 & 2: Basic extraction and creating destination directory
    std::cout << "\n--- Test Case 1 & 2: Basic Extraction ---" << std::endl;
    {
        const std::string zipFile = "archive1.zip";
        cleanup(zipFile);
        system("mkdir -p test_setup/dir1 && echo 'hello' > test_setup/file1.txt && echo 'world' > test_setup/dir1/file2.txt");
        setup("zip -r -q " + zipFile + " test_setup/*");
        
        if (extractZip(zipFile, destDir)) {
            if (checkFileContent(destPath / "test_setup/file1.txt", "hello") && checkFileContent(destPath / "test_setup/dir1/file2.txt", "world")) {
                std::cout << "PASS: Files extracted successfully." << std::endl;
            } else {
                std::cout << "FAIL: File extraction verification failed." << std::endl;
            }
        } else {
             std::cout << "FAIL: Extraction function returned false." << std::endl;
        }
        cleanup(zipFile);
    }

    // Test Case 3: Non-existent zip file
    std::cout << "\n--- Test Case 3: Non-existent ZIP file ---" << std::endl;
    {
        if (!extractZip("nonexistent.zip", destDir)) {
            std::cout << "PASS: Correctly handled non-existent zip file." << std::endl;
        } else {
            std::cout << "FAIL: Should have returned false for non-existent file." << std::endl;
        }
    }

    // Test Case 4: Extracting an empty zip file
    std::cout << "\n--- Test Case 4: Empty ZIP file ---" << std::endl;
    {
        const std::string zipFile = "archive_empty.zip";
        cleanup(zipFile);
        system("mkdir test_setup");
        setup("zip -r -q " + zipFile + " test_setup --filesync && rm -r test_setup"); // Create zip from empty dir
        
        if (extractZip(zipFile, destDir)) {
            if (fs::is_empty(destPath / "test_setup")) {
                 std::cout << "PASS: Empty zip extracted correctly." << std::endl;
            } else {
                 std::cout << "FAIL: Directory should be empty after extraction." << std::endl;
            }
        } else {
             std::cout << "FAIL: Extraction function returned false." << std::endl;
        }
        cleanup(zipFile);
    }
    
    // Test Case 5: Overwriting existing files
    std::cout << "\n--- Test Case 5: Overwriting existing files ---" << std::endl;
    {
        const std::string zipFile = "archive_overwrite.zip";
        cleanup(zipFile);
        fs::create_directories(destPath / "test_setup");
        std::ofstream(destPath / "test_setup/file.txt") << "old content";

        system("mkdir -p test_setup && echo 'new content' > test_setup/file.txt");
        setup("zip -r -q " + zipFile + " test_setup/*");
        
        if (extractZip(zipFile, destDir)) {
             if (checkFileContent(destPath / "test_setup/file.txt", "new content")) {
                 std::cout << "PASS: File was successfully overwritten." << std::endl;
             } else {
                 std::cout << "FAIL: File content was not overwritten." << std::endl;
             }
        } else {
            std::cout << "FAIL: Extraction function returned false." << std::endl;
        }
        cleanup(zipFile);
    }

    return 0;
}