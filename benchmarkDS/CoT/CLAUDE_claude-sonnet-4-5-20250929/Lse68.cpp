
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sqlite3.h>
#include <cstring>
#include <sys/stat.h>

const long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {"jpg", "jpeg", "png", "gif"};

// Base64 encoding function
std::string base64_encode(const std::vector<unsigned char>& data) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string encoded;
    int i = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    
    for (unsigned char c : data) {
        array3[i++] = c;
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for (i = 0; i < 4; i++)
                encoded += base64_chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for (int j = i; j < 3; j++)
            array3[j] = '\\0';
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            encoded += base64_chars[array4[j]];
        
        while (i++ < 3)
            encoded += '=';
    }
    
    return encoded;
}

std::string getFileExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos + 1);
}

std::string sanitizeFilename(const std::string& filename) {
    std::string sanitized;
    for (char c : filename) {
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            sanitized += c;
        } else {
            sanitized += '_';
        }
    }
    return sanitized;
}

std::string upload_image(const std::string& filePath, sqlite3* db) {
    // Validate file exists
    struct stat buffer;
    if (stat(filePath.c_str(), &buffer) != 0) {
        throw std::invalid_argument("Invalid file path");
    }
    
    // Validate file size
    if (buffer.st_size > MAX_FILE_SIZE) {
        throw std::invalid_argument("File size exceeds maximum allowed size");
    }
    
    // Extract and validate filename
    size_t lastSlash = filePath.find_last_of("/\\\\");\n    std::string fileName = (lastSlash != std::string::npos) ? \n        filePath.substr(lastSlash + 1) : filePath;\n    \n    std::string extension = getFileExtension(fileName);\n    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);\n    \n    if (std::find(ALLOWED_EXTENSIONS.begin(), ALLOWED_EXTENSIONS.end(), extension) \n        == ALLOWED_EXTENSIONS.end()) {\n        throw std::invalid_argument("Invalid file type");\n    }\n    \n    // Sanitize filename\n    std::string sanitizedFileName = sanitizeFilename(fileName);\n    \n    // Read file\n    std::ifstream file(filePath, std::ios::binary);\n    if (!file) {\n        throw std::runtime_error("Cannot open file");\n    }\n    \n    std::vector<unsigned char> fileContent((std::istreambuf_iterator<char>(file)),\n                                           std::istreambuf_iterator<char>());\n    file.close();\n    \n    // Encode to base64\n    std::string base64String = base64_encode(fileContent);\n    \n    // Insert into database using parameterized query\n    const char* sql = "INSERT INTO images (name, data) VALUES (?, ?)";\n    sqlite3_stmt* stmt;\n    \n    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {\n        throw std::runtime_error("Failed to prepare statement");\n    }\n    \n    sqlite3_bind_text(stmt, 1, sanitizedFileName.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_text(stmt, 2, base64String.c_str(), -1, SQLITE_TRANSIENT);\n    \n    if (sqlite3_step(stmt) != SQLITE_DONE) {\n        sqlite3_finalize(stmt);\n        throw std::runtime_error("Failed to insert data");\n    }\n    \n    sqlite3_finalize(stmt);\n    \n    return "Image uploaded successfully: " + sanitizedFileName;\n}\n\nvoid createTestFile(const std::string& filename, int size) {\n    std::ofstream file(filename, std::ios::binary);\n    std::vector<char> data(size, static_cast<char>(0xFF));\n    file.write(data.data(), size);\n    file.close();\n}\n\nint main() {\n    sqlite3* db;\n    \n    if (sqlite3_open("test.db", &db) != SQLITE_OK) {\n        std::cerr << "Cannot open database" << std::endl;\n        return 1;\n    }\n    \n    // Create table\n    const char* createTable = "CREATE TABLE IF NOT EXISTS images "\n                              "(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, data TEXT)";\n    sqlite3_exec(db, createTable, nullptr, nullptr, nullptr);\n    \n    // Test Case 1: Valid image\n    std::cout << "Test Case 1: Valid image" << std::endl;\n    createTestFile("test1.jpg", 100);\n    try {\n        std::cout << upload_image("test1.jpg", db) << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl;\n    }\n    \n    // Test Case 2: Valid PNG\n    std::cout << "\
Test Case 2: Valid PNG" << std::endl;\n    createTestFile("test2.png", 200);\n    try {\n        std::cout << upload_image("test2.png", db) << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl;\n    }\n    \n    // Test Case 3: Invalid extension\n    std::cout << "\
Test Case 3: Invalid extension" << std::endl;\n    createTestFile("test3.exe", 100);\n    try {\n        upload_image("test3.exe", db);\n    } catch (const std::exception& e) {\n        std::cout << "Expected error: " << e.what() << std::endl;\n    }\n    \n    // Test Case 4: Non-existent file\n    std::cout << "\
Test Case 4: Non-existent file" << std::endl;\n    try {\n        upload_image("nonexistent.jpg", db);\n    } catch (const std::exception& e) {\n        std::cout << "Expected error: " << e.what() << std::endl;\n    }\n    \n    // Test Case 5: File with special characters\n    std::cout << "\
Test Case 5: File with special characters" << std::endl;\n    createTestFile("test@#$5.jpg", 100);\n    try {\n        std::cout << upload_image("test@#$5.jpg", db) << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl;
    }
    
    sqlite3_close(db);
    return 0;
}
