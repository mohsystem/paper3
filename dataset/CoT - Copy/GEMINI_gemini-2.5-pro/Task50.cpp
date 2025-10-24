#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <random>
#include <chrono>

// C++17 <filesystem> is required. Compile with: g++ -std=c++17 your_file.cpp

namespace fs = std::filesystem;

// Define a safe directory for uploads.
const std::string UPLOAD_DIRECTORY = "uploads_cpp";
// Define a maximum file size to prevent denial-of-service attacks (e.g., 5MB).
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024;

/**
 * Simulates uploading a file to the server.
 *
 * @param originalFileName The name of the file provided by the user.
 * @param fileContent      The binary content of the file.
 * @return A confirmation or error message.
 */
std::string uploadFile(const std::string& originalFileName, const std::vector<char>& fileContent) {
    // Security: Check for empty inputs
    if (originalFileName.empty()) {
        return "Error: File name cannot be empty.";
    }
    if (fileContent.empty()) {
        return "Error: File content cannot be empty.";
    }

    // Security: Check file size
    if (fileContent.size() > MAX_FILE_SIZE) {
        return "Error: File size exceeds the " + std::to_string(MAX_FILE_SIZE / (1024 * 1024)) + "MB limit.";
    }

    try {
        // Security: Prevent path traversal attacks.
        // fs::path(...).filename() extracts just the filename part.
        fs::path originalPath(originalFileName);
        std::string sanitizedFileName = originalPath.filename().string();
        
        if (sanitizedFileName.empty() || sanitizedFileName == "." || sanitizedFileName == "..") {
            return "Error: Invalid file name provided.";
        }

        // Security: Generate a unique filename to prevent overwrites.
        auto now = std::chrono::system_clock::now();
        auto epoch = now.time_since_epoch();
        auto value = std::chrono::duration_cast<std::chrono::milliseconds>(epoch);
        std::string uniquePrefix = std::to_string(value.count());
        std::string uniqueFileName = uniquePrefix + "_" + sanitizedFileName;
        
        // Create the upload directory if it doesn't exist
        fs::path uploadPath(UPLOAD_DIRECTORY);
        if (!fs::exists(uploadPath)) {
            fs::create_directories(uploadPath);
        }

        // Construct the final, safe destination path
        fs::path destinationPath = uploadPath / uniqueFileName;

        // Write the file. The ofstream destructor will handle closing the file.
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile) {
            return "Error: Could not open file for writing.";
        }
        
        outFile.write(fileContent.data(), fileContent.size());
        
        if (!outFile.good()) {
             return "Error: A failure occurred while writing the file.";
        }

        return "Success: File '" + originalFileName + "' uploaded as '" + uniqueFileName + "'.";

    } catch (const fs::filesystem_error& e) {
        // Log the exception in a real application
        return "Error: Filesystem error. " + std::string(e.what());
    } catch (const std::exception& e) {
        // Catch any other unexpected errors
        return "Error: An unexpected error occurred. " + std::string(e.what());
    }
}

class Task50 {
public:
    static void main() {
        std::cout << "--- Running C++ Test Cases ---" << std::endl;

        // Test Case 1: Normal successful upload
        std::vector<char> content1 = {'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 't', 'e', 's', 't', '.'};
        std::cout << "Test 1 (Normal): " << uploadFile("report.txt", content1) << std::endl;

        // Test Case 2: Uploading a file with the same name (should not overwrite)
        std::vector<char> content2 = {'A', 'n', 'o', 't', 'h', 'e', 'r', ' ', 't', 'e', 's', 't', '.'};
        std::cout << "Test 2 (Duplicate Name): " << uploadFile("report.txt", content2) << std::endl;

        // Test Case 3: Path traversal attack attempt
        std::vector<char> content3 = {'x'};
        std::cout << "Test 3 (Path Traversal): " << uploadFile("../../etc/passwd", content3) << std::endl;

        // Test Case 4: File size too large
        std::vector<char> largeContent(MAX_FILE_SIZE + 1, 'A');
        std::cout << "Test 4 (Oversized File): " << uploadFile("large_file.zip", largeContent) << std::endl;

        // Test Case 5: File with spaces and special characters in name
        std::vector<char> content5 = {'f', 'i', 'n', 'a', 'l'};
        std::cout << "Test 5 (Special Chars): " << uploadFile("My Report (Final Version).docx", content5) << std::endl;
        
        std::cout << "------------------------------" << std::endl;
    }
};

int main() {
    Task50::main();
    return 0;
}