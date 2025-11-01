
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sqlite3.h>

// Maximum file size to prevent DOS attacks (5MB)
const size_t MAX_FILE_SIZE = 5 * 1024 * 1024;

// Base64 encoding function
std::string base64_encode(const unsigned char* data, size_t len) {
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string result;
    result.reserve(((len + 2) / 3) * 4);
    
    size_t i = 0;
    while (i < len) {
        unsigned char byte1 = data[i++];
        unsigned char byte2 = (i < len) ? data[i++] : 0;
        unsigned char byte3 = (i < len) ? data[i++] : 0;
        
        result += base64_chars[byte1 >> 2];
        result += base64_chars[((byte1 & 0x03) << 4) | (byte2 >> 4)];
        result += (i > len + 1) ? '=' : base64_chars[((byte2 & 0x0F) << 2) | (byte3 >> 6)];
        result += (i > len) ? '=' : base64_chars[byte3 & 0x3F];
    }
    
    return result;
}

// Validates file is actually an image by checking magic bytes
bool isValidImageFile(const std::vector<unsigned char>& data) {
    if (data.empty() || data.size() < 4) {
        return false;
    }
    
    // Check JPEG magic bytes
    if (data.size() >= 2 && data[0] == 0xFF && data[1] == 0xD8) {
        return true;
    }
    // Check PNG magic bytes
    if (data.size() >= 8 && data[0] == 0x89 && data[1] == 0x50 && 
        data[2] == 0x4E && data[3] == 0x47) {
        return true;
    }
    // Check GIF magic bytes
    if (data.size() >= 6 && data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46) {
        return true;
    }
    
    return false;
}

// Extracts basename from path to prevent path traversal
std::string getBasename(const std::string& path) {
    size_t pos = path.find_last_of("/\\\\");\n    if (pos != std::string::npos) {\n        return path.substr(pos + 1);\n    }\n    return path;\n}\n\n// Secure database connection using environment variable\nsqlite3* getSecureConnection() {\n    const char* db_path = std::getenv("DB_PATH");\n    if (!db_path) {\n        return nullptr;\n    }\n    \n    sqlite3* db = nullptr;\n    int rc = sqlite3_open(db_path, &db);\n    if (rc != SQLITE_OK) {\n        if (db) {\n            sqlite3_close(db);\n        }\n        return nullptr;\n    }\n    \n    return db;\n}\n\nstd::string uploadImage(const std::string& filename, const std::vector<unsigned char>& imageData) {\n    // Input validation: check filename is not empty\n    if (filename.empty()) {\n        return "Error: Invalid filename";\n    }\n    \n    // Input validation: check image data is not empty\n    if (imageData.empty()) {\n        return "Error: No image data provided";\n    }\n    \n    // Validate file size to prevent DOS attacks\n    if (imageData.size() > MAX_FILE_SIZE) {\n        return "Error: File size exceeds limit";\n    }\n    \n    // Sanitize filename: extract basename to prevent path traversal\n    std::string sanitizedFilename = getBasename(filename);\n    \n    // Validate file extension\n    std::regex allowedExtensions(".*\\\\.(jpg|jpeg|png|gif)$", std::regex::icase);\n    if (!std::regex_match(sanitizedFilename, allowedExtensions)) {\n        return "Error: Invalid file extension";\n    }\n    \n    // Validate actual file content by magic bytes\n    if (!isValidImageFile(imageData)) {\n        return "Error: Invalid image format";\n    }\n    \n    // Convert to base64\n    std::string base64Image = base64_encode(imageData.data(), imageData.size());\n    \n    // Insert into database using parameterized query to prevent SQL injection\n    sqlite3* db = getSecureConnection();\n    if (!db) {\n        return "Error: Database connection failed";\n    }\n    \n    sqlite3_stmt* stmt = nullptr;\n    const char* sql = "INSERT INTO images (name, base64_data) VALUES (?, ?)";\n    \n    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);\n    if (rc != SQLITE_OK) {\n        sqlite3_close(db);\n        return "Error: Failed to prepare statement";\n    }\n    \n    // Bind parameters - prevents SQL injection\n    sqlite3_bind_text(stmt, 1, sanitizedFilename.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_text(stmt, 2, base64Image.c_str(), -1, SQLITE_TRANSIENT);\n    \n    rc = sqlite3_step(stmt);\n    sqlite3_finalize(stmt);\n    sqlite3_close(db);\n    \n    if (rc == SQLITE_DONE) {\n        return "Success: Image uploaded";\n    } else {\n        return "Error: Database operation failed";\n    }\n}\n\nint main() {\n    // Test case 1: Valid JPEG image\n    std::vector<unsigned char> validJpeg(100, 0);\n    validJpeg[0] = 0xFF;\n    validJpeg[1] = 0xD8;\n    std::cout << "Test 1: " << uploadImage("test1.jpg", validJpeg) << std::endl;\n    \n    // Test case 2: Valid PNG image\n    std::vector<unsigned char> validPng(100, 0);\n    validPng[0] = 0x89;\n    validPng[1] = 0x50;\n    validPng[2] = 0x4E;\n    validPng[3] = 0x47;\n    std::cout << "Test 2: " << uploadImage("test2.png", validPng) << std::endl;\n    \n    // Test case 3: Invalid extension\n    std::cout << "Test 3: " << uploadImage("test3.exe", validJpeg) << std::endl;\n    \n    // Test case 4: Path traversal attempt\n    std::cout << "Test 4: " << uploadImage("../../etc/passwd.jpg", validJpeg) << std::endl;\n    \n    // Test case 5: Empty input\n    std::vector<unsigned char> emptyData;\n    std::cout << "Test 5: " << uploadImage("", emptyData) << std::endl;
    
    return 0;
}
