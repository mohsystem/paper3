#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // C++17 required

// For cross-platform compatibility, use std::filesystem
namespace fs = std::filesystem;

// Define a secure location for uploads
const std::string UPLOAD_DIRECTORY = "uploads";
// Define a maximum file size to prevent Denial of Service (DoS) attacks (e.g., 10MB)
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;

/**
 * Accepts a PDF file's name and content, validates it, and saves it to a
 * predefined 'uploads' folder.
 *
 * @param originalFilename The original filename from the user.
 * @param content The file content as a vector of chars.
 * @return A message indicating the result of the operation.
 */
std::string savePdfFile(const std::string& originalFilename, const std::vector<char>& content) {
    // 1. Security Check: File Size
    if (content.size() > MAX_FILE_SIZE) {
        return "Error: File size exceeds the limit of " + std::to_string(MAX_FILE_SIZE / (1024 * 1024)) + " MB.";
    }
    if (content.empty()) {
        return "Error: File content cannot be empty.";
    }

    // 2. Security Check: Filename Validation
    if (originalFilename.empty()) {
        return "Error: Filename is missing.";
    }

    // Use std::filesystem::path to safely handle file paths
    fs::path path_obj(originalFilename);
    
    // Strip path information to prevent Path Traversal attacks
    std::string filename = path_obj.filename().string();
    
    // 3. Security Check: File Type
    if (path_obj.extension() != ".pdf") {
        return "Error: Invalid file type. Only PDF files are allowed.";
    }
    
    // 4. Security Check: Sanitize filename characters
    std::string sanitizedFilename;
    for (char c : filename) {
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            sanitizedFilename += c;
        }
    }

    if (sanitizedFilename.empty() || sanitizedFilename == ".pdf") {
        return "Error: Invalid filename after sanitization.";
    }

    try {
        // Create the upload directory if it doesn't exist.
        fs::create_directories(UPLOAD_DIRECTORY);
        
        // Construct the final destination path securely
        fs::path destPath = fs::path(UPLOAD_DIRECTORY) / sanitizedFilename;

        // Security check: ensure the resolved path is still within the upload directory
        if (fs::absolute(destPath).string().find(fs::absolute(UPLOAD_DIRECTORY).string()) != 0) {
            return "Error: Directory traversal attempt detected after path resolution.";
        }
        
        // Write the file in binary mode
        std::ofstream outFile(destPath, std::ios::binary);
        if (!outFile.is_open()) {
            return "Error: Could not open file for writing.";
        }
        
        outFile.write(content.data(), content.size());
        outFile.close();
        
        return "Success: File '" + sanitizedFilename + "' saved successfully in '" + UPLOAD_DIRECTORY + "' folder.";

    } catch (const fs::filesystem_error& e) {
        // Log the exception properly in a real application
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
        return "Error: Could not save the file due to a filesystem error.";
    }
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    std::string content_str = "This is a dummy PDF content.";
    std::vector<char> validContent(content_str.begin(), content_str.end());
    std::vector<char> largeContent(MAX_FILE_SIZE + 1, 'a');

    // Test Case 1: Valid PDF file
    std::cout << "Test 1 (Valid): " << savePdfFile("MyReport_2023.pdf", validContent) << std::endl;

    // Test Case 2: Path Traversal attempt
    std::cout << "Test 2 (Path Traversal): " << savePdfFile("../../etc/passwd.pdf", validContent) << std::endl;
    
    // Test Case 3: Invalid file type
    std::cout << "Test 3 (Invalid Type): " << savePdfFile("image.jpg", validContent) << std::endl;
    
    // Test Case 4: Filename with malicious/invalid characters
    std::cout << "Test 4 (Invalid Chars): " << savePdfFile("a<b>c|d/e?f.pdf", validContent) << std::endl;
    
    // Test Case 5: File too large
    std::cout << "Test 5 (Too Large): " << savePdfFile("large_document.pdf", largeContent) << std::endl;
    std::cout << "------------------------------" << std::endl << std::endl;
    return 0;
}