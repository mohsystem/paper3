#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>
#include "sqlite3.h"

// Note: This code requires linking against the SQLite3 library.
// For example, on Linux/macOS: g++ your_file.cpp -o your_app -lsqlite3 -std=c++17
// Also requires C++17 for std::filesystem.

static const long long MAX_FILE_SIZE = 5 * 1024 * 1024; // 5 MB

// Base64 encoding function
std::string base64_encode(const std::vector<char>& in) {
    const std::string chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string out;
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

/**
 * Reads an image, converts it to Base64, and inserts it into the database.
 *
 * @param db The SQLite3 database handle.
 * @param filePathStr The path to the image file.
 * @return true if the upload was successful, false otherwise.
 */
bool uploadImage(sqlite3* db, const std::string& filePathStr) {
    std::filesystem::path imagePath(filePathStr);
    
    // Rule #1, #5: Input validation. Reject paths with directory components.
    if (imagePath.has_parent_path() || filePathStr.empty()) {
        std::cerr << "Error: Invalid file path '" << filePathStr << "'. Path traversal detected." << std::endl;
        return false;
    }

    std::error_code ec;
    if (!std::filesystem::exists(imagePath, ec) || !std::filesystem::is_regular_file(imagePath, ec)) {
        std::cerr << "Error: File does not exist or is not a regular file." << std::endl;
        return false;
    }

    uintmax_t fileSize = std::filesystem::file_size(imagePath, ec);
    if (ec) {
        std::cerr << "Error getting file size: " << ec.message() << std::endl;
        return false;
    }

    if (fileSize == 0) {
        std::cerr << "Error: File is empty." << std::endl;
        return false;
    }
    if (fileSize > MAX_FILE_SIZE) {
        std::cerr << "Error: File size " << fileSize << " bytes exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        return false;
    }

    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filePathStr << std::endl;
        return false;
    }

    std::vector<char> buffer(fileSize);
    if (!file.read(buffer.data(), fileSize)) {
        std::cerr << "Error: Could not read file " << filePathStr << std::endl;
        return false;
    }

    std::string encodedString = base64_encode(buffer);
    
    // Rule #2: Use prepared statements to prevent SQL injection.
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "INSERT INTO images(name, data) VALUES(?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    std::string fileName = imagePath.filename().string();
    sqlite3_bind_text(stmt, 1, fileName.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, encodedString.c_str(), -1, SQLITE_STATIC);

    bool success = false;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        std::cout << "Successfully uploaded " << fileName << std::endl;
        success = true;
    } else {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

void createDummyFile(const std::string& filename, size_t size) {
    std::ofstream ofs(filename, std::ios::binary | std::ios::out);
    if (!ofs) return;
    if (size > 0) {
        std::vector<char> buffer(size);
        for(size_t i = 0; i < size; ++i) buffer[i] = static_cast<char>(rand() % 256);
        ofs.write(buffer.data(), size);
    }
}

void verifyDbContent(sqlite3* db, int expectedRows) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT COUNT(*) FROM images";
    int count = -1;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    std::cout << "\nVerification: Found " << count << " rows in the database. Expected " << expectedRows << "." << std::endl;
    if (count == expectedRows) {
        std::cout << "Verification successful." << std::endl;
    } else {
        std::cout << "Verification failed." << std::endl;
    }
}


void runTestCases(sqlite3* db) {
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid small PNG file
    const std::string validFile = "test.png";
    createDummyFile(validFile, 1024);
    std::cout << "\n[Test 1] Uploading a valid small image..." << std::endl;
    uploadImage(db, validFile);

    // Test Case 2: Valid small JPG file
    const std::string validFile2 = "image.jpg";
    createDummyFile(validFile2, 2048);
    std::cout << "\n[Test 2] Uploading another valid small image..." << std::endl;
    uploadImage(db, validFile2);

    // Test Case 3: Zero-byte file
    const std::string zeroByteFile = "empty.gif";
    createDummyFile(zeroByteFile, 0);
    std::cout << "\n[Test 3] Uploading a zero-byte file (should fail)..." << std::endl;
    uploadImage(db, zeroByteFile);

    // Test Case 4: File too large
    const std::string largeFile = "large_file.bmp";
    createDummyFile(largeFile, MAX_FILE_SIZE + 1);
    std::cout << "\n[Test 4] Uploading a file that is too large (should fail)..." << std::endl;
    uploadImage(db, largeFile);
    
    // Test Case 5: Non-existent file
    std::cout << "\n[Test 5] Uploading a non-existent file (should fail)..." << std::endl;
    uploadImage(db, "nonexistent.jpg");

    verifyDbContent(db, 2);

    // Cleanup
    std::filesystem::remove(validFile);
    std::filesystem::remove(validFile2);
    std::filesystem::remove(zeroByteFile);
    std::filesystem::remove(largeFile);
    std::cout << "\n--- Test Cases Finished ---" << std::endl;
}

int main() {
    sqlite3* db;
    if (sqlite3_open(":memory:", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    std::cout << "Connected to in-memory SQLite database." << std::endl;

    char* errMsg = 0;
    const char* sql_create = "CREATE TABLE images("
                             "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "NAME TEXT NOT NULL, "
                             "DATA TEXT NOT NULL);";

    if (sqlite3_exec(db, sql_create, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }
    std::cout << "Table 'images' created." << std::endl;

    runTestCases(db);

    sqlite3_close(db);
    return 0;
}