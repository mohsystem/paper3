#include <iostream>
#include <fstream>
#include <string>
#include <filesystem> // Requires C++17 or later

// For cross-platform compatibility of path separators
#ifdef _WIN32
const char PATH_SEPARATOR = '\\';
#else
const char PATH_SEPARATOR = '/';
#endif

namespace fs = std::filesystem;

/**
 * @brief Simulates a file upload by copying a file.
 * 
 * @param sourceFilePath The path of the file to "upload".
 * @param destinationPath The destination path for the file. Can be a directory or a full path.
 * @return true if the copy was successful, false otherwise.
 */
bool uploadFile(const std::string& sourceFilePath, const std::string& destinationPath) {
    fs::path source(sourceFilePath);
    fs::path dest(destinationPath);

    if (!fs::exists(source)) {
        std::cerr << "Error: Source file does not exist: " << sourceFilePath << std::endl;
        return false;
    }
    
    if (fs::is_directory(source)) {
        std::cerr << "Error: Source path is a directory, not a file." << std::endl;
        return false;
    }

    try {
        // Ensure destination directory exists
        fs::path dest_dir = dest;
        if (dest.has_filename()){
            dest_dir = dest.parent_path();
        }

        if (!dest_dir.empty() && !fs::exists(dest_dir)) {
            fs::create_directories(dest_dir);
            std::cout << "Created directory: " << dest_dir.string() << std::endl;
        }

        // Copy with overwrite option
        fs::copy(source, dest, fs::copy_options::overwrite_existing);
        std::cout << "File uploaded successfully from '" << sourceFilePath << "' to '" << destinationPath << "'" << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "File upload failed: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    // Note: This code requires C++17. Compile with:
    // g++ -std=c++17 your_file.cpp -o your_app -lstdc++fs
    // or on MSVC, just ensure C++17 standard is set.

    // Setup for test cases
    std::string testDir = "cpp_upload_test_dir";
    std::string sourceFile = "cpp_source.txt";

    // Create a dummy source file
    {
        std::ofstream outfile(sourceFile);
        outfile << "This is a C++ test file for upload.";
    }

    // Create destination directory
    fs::create_directories(testDir);

    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful upload to a directory
    std::cout << "\n--- Test Case 1: Successful Upload to Directory ---" << std::endl;
    uploadFile(sourceFile, testDir);

    // Test Case 2: Uploading a non-existent source file
    std::cout << "\n--- Test Case 2: Non-existent Source File ---" << std::endl;
    uploadFile("non_existent_file.txt", testDir);

    // Test Case 3: Overwriting an existing file
    std::cout << "\n--- Test Case 3: Overwriting an Existing File ---" << std::endl;
    std::string overwriteFilePath = testDir + PATH_SEPARATOR + "overwrite.txt";
    uploadFile(sourceFile, overwriteFilePath); // First time
    std::cout << "Uploading again to the same destination..." << std::endl;
    uploadFile(sourceFile, overwriteFilePath); // Second time

    // Test Case 4: Uploading to a nested directory that doesn't exist
    std::cout << "\n--- Test Case 4: Upload to a non-existent nested directory ---" << std::endl;
    std::string nestedDestPath = testDir + PATH_SEPARATOR + "new" + PATH_SEPARATOR + "nested" + PATH_SEPARATOR + "uploaded_file4.txt";
    uploadFile(sourceFile, nestedDestPath);

    // Test Case 5: Uploading to a destination with a different name
    std::cout << "\n--- Test Case 5: Uploading with a new filename ---" << std::endl;
    std::string newNamePath = testDir + PATH_SEPARATOR + "renamed_file.txt";
    uploadFile(sourceFile, newNamePath);

    // Cleanup
    std::cout << "\n--- Cleaning up test files and directories ---" << std::endl;
    fs::remove(sourceFile);
    fs::remove_all(testDir);
    std::cout << "Cleanup complete." << std::endl;

    return 0;
}