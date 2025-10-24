#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <regex>
#include <algorithm>

namespace fs = std::filesystem;

const long long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {".txt", ".jpg", ".png", ".pdf"};

/**
 * @brief Securely uploads a file by copying it from a source to a destination.
 * 
 * @param sourcePathStr The path to the source file.
 * @param destDirStr The path to the destination directory.
 * @return true if upload is successful, false otherwise.
 */
bool secureFileUpload(const std::string& sourcePathStr, const std::string& destDirStr) {
    try {
        fs::path sourcePath(sourcePathStr);
        fs::path destDirPath(destDirStr);
        std::error_code ec;

        // 1. Source File Checks
        if (!fs::exists(sourcePath, ec) || !fs::is_regular_file(sourcePath, ec)) {
            std::cerr << "Error: Source file is not a valid, regular file." << std::endl;
            return false;
        }

        // 2. Destination Directory Checks
        if (!fs::exists(destDirPath, ec) || !fs::is_directory(destDirPath, ec)) {
            std::cerr << "Error: Destination is not a valid directory." << std::endl;
            return false;
        }

        // 3. File Size Check
        uintmax_t fileSize = fs::file_size(sourcePath, ec);
        if (ec) {
            std::cerr << "Error: Could not determine file size. " << ec.message() << std::endl;
            return false;
        }
        if (fileSize > MAX_FILE_SIZE) {
            std::cerr << "Error: File size exceeds the maximum limit of " 
                      << (MAX_FILE_SIZE / (1024 * 1024)) << " MB." << std::endl;
            return false;
        }

        // 4. Filename and Extension Validation
        std::string filename = sourcePath.filename().string();
        
        if (filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
            std::cerr << "Error: Filename contains invalid path characters." << std::endl;
            return false;
        }
        
        std::string file_ext = sourcePath.extension().string();
        std::transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);
        bool extensionAllowed = false;
        for (const auto& ext : ALLOWED_EXTENSIONS) {
            if (file_ext == ext) {
                extensionAllowed = true;
                break;
            }
        }
        if (!extensionAllowed) {
            std::cerr << "Error: File extension is not allowed." << std::endl;
            return false;
        }

        // 5. Path Traversal Check
        fs::path canonicalDestDir = fs::canonical(destDirPath, ec);
        if(ec) {
            std::cerr << "Error: Could not resolve destination directory path. " << ec.message() << std::endl;
            return false;
        }
        fs::path destFilePath = canonicalDestDir / filename;
        fs::path finalDestPath = fs::weakly_canonical(destFilePath, ec);
        
        if (finalDestPath.parent_path() != canonicalDestDir) {
            std::cerr << "Error: Path traversal attempt detected." << std::endl;
            return false;
        }

        // 6. Check for existing file
        if (fs::exists(destFilePath, ec)) {
            std::cerr << "Error: File with the same name already exists in the destination." << std::endl;
            return false;
        }

        // 7. Perform the copy
        fs::copy(sourcePath, destFilePath, ec);
        if (ec) {
            std::cerr << "Error: File copy failed. " << ec.message() << std::endl;
            return false;
        }

        std::cout << "Success: File '" << filename << "' uploaded successfully." << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
        return false;
    }
}

void create_test_file(const std::string& path, const std::string& content) {
    std::ofstream outfile(path);
    outfile << content;
}

void create_large_test_file(const std::string& path, long long size) {
    std::ofstream outfile(path, std::ios::binary);
    std::vector<char> buffer(1024 * 4, 0);
    for (long long i = 0; i < size; i += buffer.size()) {
        outfile.write(buffer.data(), std::min((long long)buffer.size(), size - i));
    }
}

int main() {
    // Setup for tests
    std::string uploadDir = "uploads_cpp";
    fs::create_directories(uploadDir);

    create_test_file("test_good.txt", "This is a good file.");
    create_large_test_file("test_large.bin", MAX_FILE_SIZE + 1);
    create_test_file("test_bad.exe", "dummy executable");
    create_test_file("traversal_test.txt", "This file should not escape the upload dir");

    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Successful upload
    std::cout << "\n[Test 1] Successful Upload:" << std::endl;
    secureFileUpload("test_good.txt", uploadDir);

    // Test Case 2: File already exists
    std::cout << "\n[Test 2] File Exists:" << std::endl;
    secureFileUpload("test_good.txt", uploadDir);

    // Test Case 3: File too large
    std::cout << "\n[Test 3] File Too Large:" << std::endl;
    secureFileUpload("test_large.bin", uploadDir);

    // Test Case 4: Disallowed extension
    std::cout << "\n[Test 4] Disallowed Extension:" << std::endl;
    secureFileUpload("test_bad.exe", uploadDir);

    // Test Case 5: Path Traversal Attempt
    std::cout << "\n[Test 5] Path Traversal Attempt:" << std::endl;
    std::string maliciousDestDir = uploadDir + "/../";
    secureFileUpload("traversal_test.txt", maliciousDestDir);

    // Cleanup
    fs::remove("test_good.txt");
    fs::remove("test_large.bin");
    fs::remove("test_bad.exe");
    fs::remove("traversal_test.txt");
    return 0;
}