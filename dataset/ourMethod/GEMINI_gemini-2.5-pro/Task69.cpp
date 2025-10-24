#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

/**
 * @brief Deletes a file at the given path after performing security checks.
 *        It will not delete directories or symbolic links.
 * 
 * @param file_path_str The path to the file to be deleted.
 * @return true if the file was successfully deleted, false otherwise.
 */
bool deleteFile(const std::string& file_path_str) {
    if (file_path_str.empty()) {
        std::cerr << "Error: File path cannot be empty." << std::endl;
        return false;
    }

    std::error_code ec;
    fs::path path(file_path_str);
    
    // Use symlink_status to check the status of the link itself, not its target.
    // This is crucial for mitigating TOCTOU vulnerabilities.
    fs::file_status status = fs::symlink_status(path, ec);
    if (ec) {
        std::cerr << "Error: Cannot access path status: " << path.string() << ". " << ec.message() << std::endl;
        return false;
    }

    if (fs::is_symlink(status)) {
        std::cerr << "Error: Deleting symbolic links is not allowed. Path: " << path.string() << std::endl;
        return false;
    }

    if (!fs::exists(status)) {
        std::cerr << "Error: File does not exist. Path: " << path.string() << std::endl;
        return false;
    }

    if (fs::is_directory(status)) {
        std::cerr << "Error: Path is a directory, not a file. Path: " << path.string() << std::endl;
        return false;
    }

    if (!fs::is_regular_file(status)) {
        std::cerr << "Error: Path is not a regular file. Path: " << path.string() << std::endl;
        return false;
    }
    
    if (fs::remove(path, ec)) {
        std::cout << "Successfully deleted file: " << path.string() << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to delete file " << path.string() << ". " << ec.message() << std::endl;
        return false;
    }
}

void runTest(const std::string& testName, const std::string& path) {
    std::cout << "--- " << testName << " ---" << std::endl;
    std::cout << "Attempting to delete: " << path << std::endl;
    deleteFile(path);
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        return deleteFile(argv[1]) ? 0 : 1;
    } else {
        std::cout << "Usage: " << argv[0] << " <file_path>" << std::endl;
        std::cout << "\n--- No file path provided, running built-in test cases ---" << std::endl;

        const fs::path testFile("test_file_cpp.txt");
        const fs::path testDir("test_dir_cpp");
        const fs::path testLink("test_link_cpp.txt");
        const fs::path nonExistentFile("non_existent_file.txt");
        std::error_code ec;
        
        fs::remove(testFile, ec);
        fs::remove(testLink, ec);
        fs::remove_all(testDir, ec);

        std::ofstream(testFile).close();
        fs::create_directory(testDir, ec);
        fs::create_symlink(testFile, testLink, ec);
        if (ec) {
            std::cerr << "Warning: Could not create symbolic link for test. " << ec.message() << std::endl;
        }

        runTest("Test Case 1: Delete existing file", testFile.string());
        runTest("Test Case 2: Delete non-existent file", nonExistentFile.string());
        runTest("Test Case 3: Delete a directory", testDir.string());
        if (fs::is_symlink(testLink)) {
            runTest("Test Case 4: Delete a symbolic link", testLink.string());
        } else {
             std::cout << "--- Skipping Test Case 4: Symbolic link was not created ---\n" << std::endl;
        }
        runTest("Test Case 5: Re-delete already deleted file", testFile.string());

        fs::remove(testFile, ec);
        fs::remove(testLink, ec);
        fs::remove_all(testDir, ec);
        std::cout << "--- Test cleanup complete ---" << std::endl;

        return 0;
    }
}