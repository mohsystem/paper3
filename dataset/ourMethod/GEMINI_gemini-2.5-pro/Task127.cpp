#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <system_error>
#include <chrono>

namespace fs = std::filesystem;

// Function to generate a unique filename in the temp directory
std::string get_unique_temp_path() {
    auto now = std::chrono::high_resolution_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    return (fs::temp_directory_path() / ("temp-copy-" + std::to_string(nanos) + ".tmp")).string();
}

/**
 * @brief Reads a file from a base directory and writes its content to a new temporary file.
 * This function is secured against path traversal attacks.
 *
 * @param baseDirStr The trusted base directory.
 * @param fileNameStr The name of the file to be read (untrusted).
 * @return The path to the temporary file as a string, or an empty string if an error occurred.
 */
std::string copyFileToTemp(const std::string& baseDirStr, const std::string& fileNameStr) {
    try {
        if (fileNameStr.find('/') != std::string::npos || fileNameStr.find('\\') != std::string::npos) {
            std::cerr << "Error: File name cannot contain path separators." << std::endl;
            return "";
        }

        fs::path baseDirPath = fs::path(baseDirStr);
        baseDirPath = fs::canonical(baseDirPath);

        if (!fs::is_directory(baseDirPath)) {
             std::cerr << "Error: Base path is not a directory." << std::endl;
             return "";
        }
        
        fs::path inputFilePath = baseDirPath / fileNameStr;
        inputFilePath = fs::canonical(inputFilePath);

        std::string baseStr = baseDirPath.string();
        std::string inputStr = inputFilePath.string();

        if (inputStr.rfind(baseStr, 0) != 0) {
            std::cerr << "Error: Path traversal attempt detected. Access denied." << std::endl;
            return "";
        }

        if (!fs::is_regular_file(inputFilePath)) {
            std::cerr << "Error: The specified path is not a regular file." << std::endl;
            return "";
        }

        std::string tempFilePathStr = get_unique_temp_path();
        fs::path tempFilePath(tempFilePathStr);
        
        std::ifstream inFile(inputFilePath, std::ios::binary);
        if (!inFile.is_open()) {
            std::cerr << "Error: Could not open source file for reading." << std::endl;
            return "";
        }

        std::ofstream outFile(tempFilePath, std::ios::binary | std::ios::trunc);
        if (!outFile.is_open()) {
            std::cerr << "Error: Could not open temporary file for writing." << std::endl;
            return "";
        }
        
        inFile.exceptions(std::ifstream::badbit);
        outFile.exceptions(std::ofstream::badbit | std::ofstream::failbit);

        outFile << inFile.rdbuf();

        return tempFilePath.string();

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
    } catch (const std::ios_base::failure& e) {
        std::cerr << "I/O Error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    return "";
}

void setup_test_environment(const fs::path& baseDir) {
    fs::create_directory(baseDir);
    std::ofstream(baseDir / "valid.txt") << "Hello, C++!";
    fs::create_directory(baseDir / "subdir");
}

void cleanup_test_environment(const fs::path& baseDir) {
    fs::remove_all(baseDir);
}

int main() {
    fs::path testBaseDir = fs::temp_directory_path() / "test-basedir-cpp";
    setup_test_environment(testBaseDir);

    std::cout << "Test environment set up in: " << testBaseDir.string() << std::endl;
    std::cout << "------------------------------------------" << std::endl;
    
    std::cout << "Test Case 1: Valid file 'valid.txt'" << std::endl;
    std::string tempPath1 = copyFileToTemp(testBaseDir.string(), "valid.txt");
    if (!tempPath1.empty()) {
        std::cout << "Success! Copied to: " << tempPath1 << std::endl;
        fs::remove(tempPath1);
    } else {
        std::cout << "Failed unexpectedly." << std::endl;
    }
    std::cout << std::endl;

    std::cout << "Test Case 2: Non-existent file 'nonexistent.txt'" << std::endl;
    std::string tempPath2 = copyFileToTemp(testBaseDir.string(), "nonexistent.txt");
    if (tempPath2.empty()) {
        std::cout << "Success! Operation failed as expected." << std::endl;
    } else {
        std::cout << "Failed! An unexpected temporary file was created: " << tempPath2 << std::endl;
        fs::remove(tempPath2);
    }
    std::cout << std::endl;

    std::cout << "Test Case 3: Path traversal attempt '../test.txt'" << std::endl;
    std::string tempPath3 = copyFileToTemp(testBaseDir.string(), "../test.txt");
    if (tempPath3.empty()) {
        std::cout << "Success! Path traversal blocked as expected." << std::endl;
    } else {
        std::cout << "Failed! Path traversal was successful: " << tempPath3 << std::endl;
        fs::remove(tempPath3);
    }
    std::cout << std::endl;
    
    std::cout << "Test Case 4: Accessing a directory 'subdir'" << std::endl;
    std::string tempPath4 = copyFileToTemp(testBaseDir.string(), "subdir");
    if (tempPath4.empty()) {
        std::cout << "Success! Directory access blocked as expected." << std::endl;
    } else {
        std::cout << "Failed! Directory was copied: " << tempPath4 << std::endl;
        fs::remove(tempPath4);
    }
    std::cout << std::endl;

    std::cout << "Test Case 5: Using a non-existent base directory" << std::endl;
    std::string nonExistentBaseDir = (testBaseDir / "nonexistent_dir").string();
    std::string tempPath5 = copyFileToTemp(nonExistentBaseDir, "valid.txt");
    if (tempPath5.empty()) {
        std::cout << "Success! Invalid base directory handled correctly." << std::endl;
    } else {
        std::cout << "Failed! An unexpected file was created: " << tempPath5 << std::endl;
        fs::remove(tempPath5);
    }
    
    cleanup_test_environment(testBaseDir);
    return 0;
}