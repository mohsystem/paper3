#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>
#include <random>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// Generates a random hexadecimal string of a given length.
std::string generate_random_hex(size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << distrib(gen);
    }
    return ss.str();
}

/**
 * Securely "uploads" a file by copying it from a source path to a destination directory.
 * This implementation prevents path traversal and uses an atomic move to mitigate TOCTOU vulnerabilities.
 *
 * @param sourceFilePath  The path to the file to be uploaded.
 * @param destinationDir  The directory where the file will be saved.
 * @param newFileName     The name for the new file. Must not contain path separators.
 * @return true on success, false on failure.
 */
bool uploadFile(const std::string& sourceFilePath, const std::string& destinationDir, const std::string& newFileName) {
    // Rule #6: Validate inputs
    if (sourceFilePath.empty() || destinationDir.empty() || newFileName.empty()) {
        std::cerr << "Error: Input paths cannot be empty." << std::endl;
        return false;
    }

    fs::path fileNamePath(newFileName);
    // Rule #9: Prevent path traversal by validating the filename.
    if (fileNamePath.has_parent_path() || newFileName.find("..") != std::string::npos || 
        newFileName.find('/') != std::string::npos || newFileName.find('\\') != std::string::npos) {
        std::cerr << "Error: Invalid filename. It must not contain path elements." << std::endl;
        return false;
    }

    std::error_code ec;
    fs::path sourcePath(sourceFilePath);
    fs::path destDirPath(destinationDir);

    if (!fs::is_regular_file(sourcePath, ec) || ec) {
        std::cerr << "Error: Source file does not exist or is not a regular file. " << ec.message() << std::endl;
        return false;
    }
    if (!fs::is_directory(destDirPath, ec) || ec) {
        std::cerr << "Error: Destination directory does not exist or is not a directory. " << ec.message() << std::endl;
        return false;
    }
    
    // Rule #9: Resolve paths to their canonical form.
    fs::path canonicalDestDir = fs::canonical(destDirPath, ec);
    if (ec) {
        std::cerr << "Error: Could not get canonical path for destination directory. " << ec.message() << std::endl;
        return false;
    }
    fs::path finalDestPath = canonicalDestDir / fileNamePath;
    
    fs::path tempFilePath;
    try {
        // Rule #3: Mitigate TOCTOU by writing to a temporary file and then moving atomically.
        std::string tempFileName = ".tmp_" + generate_random_hex(8);
        tempFilePath = canonicalDestDir / tempFileName;

        std::ifstream sourceFile(sourcePath, std::ios::binary);
        if (!sourceFile.is_open()) {
            std::cerr << "Error: Could not open source file for reading." << std::endl;
            return false;
        }

        std::ofstream tempFile(tempFilePath, std::ios::binary | std::ios::trunc);
        if (!tempFile.is_open()) {
            std::cerr << "Error: Could not create temporary file for writing." << std::endl;
            return false;
        }

        // Rule #1, #2: Use a buffer for copying.
        std::vector<char> buffer(4096);
        while (sourceFile.read(buffer.data(), buffer.size()) || sourceFile.gcount() > 0) {
            tempFile.write(buffer.data(), sourceFile.gcount());
            if (!tempFile) {
                 std::cerr << "Error writing to temporary file." << std::endl;
                 throw std::runtime_error("Write error");
            }
        }
        sourceFile.close();
        tempFile.close();

        // Atomically rename the temporary file to the final destination.
        fs::rename(tempFilePath, finalDestPath, ec);
        if (ec) {
            std::cerr << "Error moving temporary file to final destination: " << ec.message() << std::endl;
            fs::remove(tempFilePath, ec); // Cleanup
            return false;
        }
        
        std::cout << "File uploaded successfully to: " << fs::absolute(finalDestPath).string() << std::endl;
        return true;

    } catch (const std::exception& e) {
        // Rule #15: Catch and handle exceptions
        std::cerr << "An error occurred: " << e.what() << std::endl;
        if (!tempFilePath.empty()) fs::remove(tempFilePath, ec); // Cleanup
        return false;
    }
}

int main() {
    // Setup test environment
    fs::path testRoot = fs::temp_directory_path() / ("test_root_cpp_" + generate_random_hex(4));
    fs::create_directories(testRoot);

    fs::path sourceDir = testRoot / "sources";
    fs::path uploadDir = testRoot / "uploads";
    fs::create_directories(sourceDir);
    fs::create_directories(uploadDir);

    fs::path sourceFile = sourceDir / "testfile.txt";
    {
        std::ofstream ofs(sourceFile);
        ofs << "This is a test file.";
    }

    std::cout << "Test environment created in: " << testRoot.string() << std::endl;
    std::cout << "Source file: " << sourceFile.string() << std::endl;
    std::cout << "Upload directory: " << uploadDir.string() << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Test Case 1: Successful upload
    std::cout << "Test Case 1: Successful upload" << std::endl;
    bool success1 = uploadFile(sourceFile.string(), uploadDir.string(), "newfile.txt");
    std::cout << "Result: " << (success1 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Test Case 2: Path traversal attempt
    std::cout << "Test Case 2: Path traversal attempt with '../evil.txt'" << std::endl;
    bool success2 = uploadFile(sourceFile.string(), uploadDir.string(), "../evil.txt");
    std::cout << "Result: " << (success2 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Test Case 3: Source file does not exist
    std::cout << "Test Case 3: Source file does not exist" << std::endl;
    bool success3 = uploadFile("nonexistent.txt", uploadDir.string(), "anotherfile.txt");
    std::cout << "Result: " << (success3 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Test Case 4: Destination directory does not exist
    std::cout << "Test Case 4: Destination directory does not exist" << std::endl;
    bool success4 = uploadFile(sourceFile.string(), (uploadDir / "nonexistent").string(), "anotherfile.txt");
    std::cout << "Result: " << (success4 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Test Case 5: Filename contains a subdirectory
    std::cout << "Test Case 5: Filename contains a subdirectory 'subdir/file.txt'" << std::endl;
    bool success5 = uploadFile(sourceFile.string(), uploadDir.string(), "subdir/file.txt");
    std::cout << "Result: " << (success5 ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // Cleanup
    std::error_code ec;
    fs::remove_all(testRoot, ec);
    if(ec) {
        std::cerr << "Failed to cleanup test directory " << testRoot.string() << ": " << ec.message() << std::endl;
    }

    return 0;
}