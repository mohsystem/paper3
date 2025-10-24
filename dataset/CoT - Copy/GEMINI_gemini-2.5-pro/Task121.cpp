#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>

// For directory creation and path manipulation
#if defined(_WIN32)
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// Configuration for file uploads
const std::string UPLOAD_DIRECTORY = "uploads";
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB
const std::set<std::string> ALLOWED_EXTENSIONS = {".txt", ".pdf", ".jpg", ".png"};

/**
 * Sanitizes a filename to prevent path traversal attacks.
 * @param filepath The original filename.
 * @return The sanitized basename of the file.
 */
std::string sanitizeFilename(const std::string& filepath) {
    size_t last_slash = filepath.find_last_of("/\\");
    if (std::string::npos != last_slash) {
        return filepath.substr(last_slash + 1);
    }
    return filepath;
}

/**
 * Simulates securely saving an uploaded file.
 * Performs security checks for file size, path traversal, and file extension.
 * @param originalFilename The filename provided by the user.
 * @param fileContent The byte content of the file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool saveUploadedFile(const std::string& originalFilename, const std::vector<char>& fileContent) {
    // 1. Security Check: File Size
    if (fileContent.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the " << (MAX_FILE_SIZE / 1024 / 1024) << "MB limit." << std::endl;
        return false;
    }

    // 2. Security Check: Path Traversal
    std::string sanitizedFilename = sanitizeFilename(originalFilename);
    if (sanitizedFilename.empty() || sanitizedFilename == "." || sanitizedFilename == "..") {
        std::cerr << "Error: Invalid filename provided." << std::endl;
        return false;
    }

    // 3. Security Check: File Extension Whitelist
    size_t dotIndex = sanitizedFilename.find_last_of('.');
    if (dotIndex == std::string::npos || dotIndex == 0) {
        std::cerr << "Error: File has no extension or is a hidden file." << std::endl;
        return false;
    }
    std::string extension = sanitizedFilename.substr(dotIndex);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
        std::cerr << "Error: File type '" << extension << "' is not allowed." << std::endl;
        return false;
    }

    // 4. Construct the final safe path
    MKDIR(UPLOAD_DIRECTORY.c_str()); // Create directory, ignore error if it exists
    std::string destinationPath = UPLOAD_DIRECTORY + "/" + sanitizedFilename;
    
    // Prevent overwriting existing files
    std::ifstream f(destinationPath.c_str());
    if (f.good()) {
        std::cerr << "Error: A file with this name already exists." << std::endl;
        f.close();
        return false;
    }
    f.close();

    // 5. Write the file to the secure location
    std::ofstream outFile(destinationPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Could not open file for writing." << std::endl;
        return false;
    }
    outFile.write(fileContent.data(), fileContent.size());
    outFile.close();

    if (!outFile.good()) {
        std::cerr << "Error: Failed to write file content." << std::endl;
        return false;
    }
    
    std::cout << "Success: File '" << sanitizedFilename << "' uploaded to '" << UPLOAD_DIRECTORY << "' directory." << std::endl;
    return true;
}

void runTests() {
    std::cout << "--- Running C++ File Upload Tests ---" << std::endl;
    std::vector<char> smallContent = {'t', 'e', 's', 't'};
    std::vector<char> largeContent(MAX_FILE_SIZE + 1, 'A');

    // Test Case 1: Valid file upload
    std::cout << "\n[Test Case 1: Valid file]" << std::endl;
    saveUploadedFile("report.pdf", smallContent);

    // Test Case 2: Invalid file extension
    std::cout << "\n[Test Case 2: Invalid extension]" << std::endl;
    saveUploadedFile("archive.zip", smallContent);

    // Test Case 3: Path Traversal attempt
    std::cout << "\n[Test Case 3: Path Traversal]" << std::endl;
    saveUploadedFile("..\\..\\boot.ini", smallContent);
    
    // Test Case 4: File too large
    std::cout << "\n[Test Case 4: File too large]" << std::endl;
    saveUploadedFile("huge_image.jpg", largeContent);

    // Test Case 5: Empty filename after sanitization
    std::cout << "\n[Test Case 5: Malformed name]" << std::endl;
    saveUploadedFile("../", smallContent);

    // Cleanup
    std::string file_to_remove = UPLOAD_DIRECTORY + "/report.pdf";
    remove(file_to_remove.c_str());
}

int main() {
    runTests();
    return 0;
}