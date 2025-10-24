#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include <algorithm> // for std::mismatch

// For generating a somewhat unique name for temp files
#include <chrono>
#include <random>

namespace fs = std::filesystem;

/**
 * Generates a unique filename component.
 * @return A string with a random component.
 */
std::string generateUniqueId() {
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(timestamp);
    return std::to_string(rng());
}

/**
 * Securely reads a file from a specified base directory and writes it to a temporary location.
 * Prevents path traversal attacks.
 *
 * @param baseDirStr The trusted base directory from which to read files.
 * @param fileName   The name of the file to read, relative to the base directory.
 * @return The path to the created temporary file as a string, or an empty string on failure.
 */
std::string copyFileToTemp(const std::string& baseDirStr, const std::string& fileName) {
    if (fileName.empty()) {
        std::cerr << "Error: File name cannot be empty." << std::endl;
        return "";
    }
    
    try {
        // Resolve paths and check for existence
        fs::path baseDir = fs::canonical(baseDirStr);
        fs::path sourcePath = baseDir / fileName;
        
        // Security Check: Normalize and ensure the path is within the base directory.
        sourcePath = fs::weakly_canonical(sourcePath);
        auto res = std::mismatch(baseDir.begin(), baseDir.end(), sourcePath.begin(), sourcePath.end());
        if (res.first != baseDir.end()) {
             throw std::invalid_argument("Path traversal attempt detected for file: " + fileName);
        }

        if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
            throw fs::filesystem_error("Source file does not exist or is not a regular file", sourcePath, std::make_error_code(std::errc::no_such_file_or_directory));
        }

        // Create a secure temporary file path
        fs::path tempPath = fs::temp_directory_path() / ("temp_copy_" + generateUniqueId() + ".tmp");

        // Open streams using RAII
        std::ifstream src(sourcePath, std::ios::binary);
        if (!src.is_open()) {
             throw fs::filesystem_error("Could not open source file for reading", sourcePath, std::make_error_code(std::errc::permission_denied));
        }
        
        std::ofstream dst(tempPath, std::ios::binary);
        if (!dst.is_open()) {
             throw fs::filesystem_error("Could not create temporary file for writing", tempPath, std::make_error_code(std::errc::permission_denied));
        }

        // Copy content
        dst << src.rdbuf();
        
        // Check for read/write errors
        if (src.bad() || dst.bad()) {
            fs::remove(tempPath); // Clean up failed attempt
            throw std::runtime_error("An I/O error occurred during file copy.");
        }
        
        std::cout << "Successfully copied '" << sourcePath.string() << "' to '" << tempPath.string() << "'" << std::endl;
        return tempPath.string();

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    
    return "";
}

class Task127 {
public:
    static void main() {
        // --- Setup Test Environment ---
        const std::string testDirName = "cpp_test_dir";
        fs::path goodFilePath = fs::path(testDirName) / "good_file.txt";
        fs::path emptyFilePath = fs::path(testDirName) / "empty_file.txt";
        fs::path unreadableFilePath = fs::path(testDirName) / "unreadable.txt";

        try {
            fs::create_directory(testDirName);
            
            std::ofstream(goodFilePath) << "This is a test file.";
            std::ofstream(emptyFilePath).close();
            std::ofstream(unreadableFilePath).close();
            
            // Make file unreadable
            fs::permissions(unreadableFilePath, fs::perms::none, fs::perm_options::replace);

            std::cout << "--- Running Test Cases ---" << std::endl;

            // Test Case 1: Happy path - valid file
            std::cout << "\n1. Testing with a valid file:" << std::endl;
            std::string temp1 = copyFileToTemp(testDirName, "good_file.txt");
            if (!temp1.empty()) fs::remove(temp1);

            // Test Case 2: File not found
            std::cout << "\n2. Testing with a non-existent file:" << std::endl;
            copyFileToTemp(testDirName, "non_existent_file.txt");

            // Test Case 3: Path traversal attack
            std::cout << "\n3. Testing path traversal attack:" << std::endl;
            copyFileToTemp(testDirName, "../some_other_file.txt");

            // Test Case 4: Empty file
            std::cout << "\n4. Testing with an empty file:" << std::endl;
            std::string temp4 = copyFileToTemp(testDirName, "empty_file.txt");
            if (!temp4.empty()) fs::remove(temp4);
            
            // Test Case 5: Permission denied (read)
            std::cout << "\n5. Testing with an unreadable file:" << std::endl;
            copyFileToTemp(testDirName, "unreadable.txt");
            
            std::cout << "\n--- Test Cases Finished ---" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to set up/run test environment: " << e.what() << std::endl;
        }

        // --- Cleanup Test Environment ---
        if (fs::exists(testDirName)) {
            fs::remove_all(testDirName);
            std::cout << "\nTest environment cleaned up." << std::endl;
        }
    }
};

int main() {
    Task127::main();
    return 0;
}