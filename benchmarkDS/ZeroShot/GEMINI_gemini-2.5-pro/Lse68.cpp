#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>
#include <stdexcept>

// C++17 <filesystem> is required. Compile with: g++ -std=c++17 your_file.cpp -o your_app

namespace fs = std::filesystem;

// Define a reasonable max file size to prevent DoS attacks (e.g., 5MB)
const long long MAX_FILE_SIZE = 5 * 1024 * 1024;
// Whitelist for safe characters in a filename
const std::regex SAFE_FILENAME_REGEX("^[a-zA-Z0-9._-]+$");

// Helper function to encode binary data to Base64
std::string base64_encode(const std::vector<char>& data) {
    static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t in_len = data.size();
    const char* bytes_to_encode = data.data();

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}

/**
 * Securely handles image upload: validates, reads, base64 encodes, and simulates a secure DB insert.
 * @param filePath The path to the image file provided by the user.
 * @return true if the operation was successful, false otherwise.
 */
bool uploadImage(const std::string& filePathStr) {
    std::cout << "--- Processing: " << filePathStr << " ---" << std::endl;
    try {
        // 1. Input Validation: Path and File Existence
        fs::path path(filePathStr);
        if (!fs::exists(path) || !fs::is_regular_file(path)) {
            std::cerr << "Error: File does not exist or is not a regular file." << std::endl;
            return false;
        }

        // 2. Security: Sanitize Filename
        std::string originalFilename = path.filename().string();
        if (!std::regex_match(originalFilename, SAFE_FILENAME_REGEX)) {
            std::cerr << "Error: Filename contains invalid characters." << std::endl;
            return false;
        }
        std::string sanitizedFilename = originalFilename;

        // 3. Security: Check File Size
        uintmax_t fileSize = fs::file_size(path);
        if (fileSize > MAX_FILE_SIZE) {
            std::cerr << "Error: File size (" << fileSize << " bytes) exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
            return false;
        }
        if (fileSize == 0) {
            std::cerr << "Error: File is empty." << std::endl;
            return false;
        }

        // 4. Read file and convert to Base64
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file for reading." << std::endl;
            return false;
        }
        std::vector<char> buffer(fileSize);
        if (!file.read(buffer.data(), fileSize)) {
            std::cerr << "Error: Failed to read file content." << std::endl;
            return false;
        }
        std::string base64String = base64_encode(buffer);

        // 5. Security: Simulate Secure Database Insertion
        std::cout << "Simulating secure database insert..." << std::endl;
        std::cout << "  Query Template: INSERT INTO images (name, data) VALUES (?, ?);" << std::endl;
        std::cout << "  Binding Param 1 (name): " << sanitizedFilename << std::endl;
        std::string truncatedBase64 = base64String.substr(0, 40) + (base64String.length() > 40 ? "..." : "");
        std::cout << "  Binding Param 2 (data): " << truncatedBase64 << std::endl;
        std::cout << "  Execution successful." << std::endl;
        
        return true;

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem Error: " << e.what() << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return false;
    } finally {
        std::cout << "--- Finished processing: " << filePathStr << " ---\n" << std::endl;
    }
}

// Helper to create a dummy file for testing
void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream outfile(filename);
    outfile << content;
    outfile.close();
}


int main() {
    // --- Test Case Setup ---
    const std::string validFile = "test_image.jpg";
    const std::string largeFile = "large_image.png";
    const std::string badNameFile = "bad'name;--.txt";
    createTestFile(validFile, "This is a dummy image file.");
    createTestFile(largeFile, "This file is to test the size check logic.");
    createTestFile(badNameFile, "bad content");

    // --- Running 5 Test Cases ---
    std::cout << "====== Running C++ Test Cases ======" << std::endl;
    
    // 1. Valid upload
    uploadImage(validFile);

    // 2. File does not exist
    uploadImage("non_existent_file.gif");

    // 3. File path with traversal attempt (should be sanitized by path.filename())
    uploadImage("../../dummy_dir/" + validFile);

    // 4. Filename with invalid characters
    uploadImage(badNameFile);

    // 5. File is too large (conceptual test)
    std::cout << "Testing large file (will pass if its actual size is small, but demonstrates the check is in place)..." << std::endl;
    uploadImage(largeFile);

    // --- Test Case Cleanup ---
    fs::remove(validFile);
    fs::remove(largeFile);
    fs::remove(badNameFile);

    return 0;
}