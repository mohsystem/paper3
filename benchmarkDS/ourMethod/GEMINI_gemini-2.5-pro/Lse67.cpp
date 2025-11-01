#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <memory>

// --- Dependencies ---
// Requires MySQL Connector/C++ 8.0.
// Link with: -lmysqlcppconn8
// Example compilation: g++ -std=c++17 -o store_image store_image.cpp -lmysqlcppconn8

// --- Database Schema ---
// CREATE DATABASE IF NOT EXISTS image_db;
// USE image_db;
// CREATE TABLE IF NOT EXISTS images (
//     id INT AUTO_INCREMENT PRIMARY KEY,
//     filename VARCHAR(255) NOT NULL,
//     data LONGBLOB NOT NULL,
//     created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
// );

#include <mysql/jdbc.h>

// --- Database Configuration ---
// It's recommended to load these from a secure source, not hardcoded.
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "your_user";
const std::string DB_PASS = "your_password";
const std::string DB_NAME = "image_db";

// --- Security Configuration ---
const std::filesystem::path BASE_DIRECTORY = std::filesystem::temp_directory_path() / "image_uploads_safe_dir";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

/**
 * Stores an image file in the database after performing security checks.
 *
 * @param unsafeFilePath The path to the image file.
 * @return true if successful, false otherwise.
 */
bool storeImageInDb(const std::string& unsafeFilePath) {
    if (unsafeFilePath.empty()) {
        std::cerr << "Error: File path is empty." << std::endl;
        return false;
    }

    std::filesystem::path filePath(unsafeFilePath);
    std::filesystem::path canonicalPath;

    try {
        // Rule #5: Validate path to prevent traversal.
        canonicalPath = std::filesystem::canonical(filePath);
        auto baseCanonical = std::filesystem::canonical(BASE_DIRECTORY);

        // Check if the file path is within the base directory.
        bool is_in_base = false;
        for (auto p = canonicalPath.parent_path(); p.has_parent_path(); p = p.parent_path()) {
            if (p == baseCanonical) {
                is_in_base = true;
                break;
            }
        }
        if (!is_in_base && canonicalPath.parent_path() != baseCanonical) {
            std::cerr << "Error: Path traversal attempt detected. File path is outside the allowed directory." << std::endl;
            return false;
        }

        // Rule #6: Check properties of the path after canonicalization.
        if (!std::filesystem::is_regular_file(canonicalPath)) {
            std::cerr << "Error: Path does not point to a regular file." << std::endl;
            return false;
        }

        // Rule #1 & #3: Validate file size.
        uintmax_t fileSize = std::filesystem::file_size(canonicalPath);
        if (fileSize > MAX_FILE_SIZE) {
            std::cerr << "Error: File size " << fileSize << " bytes exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
            return false;
        }

        // Read file contents
        std::ifstream fileStream(canonicalPath, std::ios::binary);
        if (!fileStream.is_open()) {
             std::cerr << "Error: Could not open file: " << canonicalPath << std::endl;
             return false;
        }

        // This approach of wrapping the stream is more efficient for large files.
        // We do not need to load the entire file into memory first.
        
        sql::Driver* driver;
        std::unique_ptr<sql::Connection> con;
        std::unique_ptr<sql::PreparedStatement> pstmt;

        try {
            driver = get_driver_instance();
            con.reset(driver->connect(DB_HOST, DB_USER, DB_PASS));
            con->setSchema(DB_NAME);

            // Rule #2: Use prepared statements.
            pstmt.reset(con->prepareStatement("INSERT INTO images (filename, data) VALUES (?, ?)"));
            pstmt->setString(1, canonicalPath.filename().string());
            pstmt->setBlob(2, &fileStream);
            
            int affectedRows = pstmt->executeUpdate();
            if (affectedRows > 0) {
                 std::cout << "Successfully stored image: " << canonicalPath.filename().string() << std::endl;
                 return true;
            } else {
                 std::cerr << "Error: Storing image failed, no rows affected." << std::endl;
            }

        } catch (const sql::SQLException& e) {
            std::cerr << "Database error: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ")" << std::endl;
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "File system error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }

    return false;
}

void setupTestEnvironment() {
    try {
        std::filesystem::create_directories(BASE_DIRECTORY);
        std::cout << "Created base directory for tests: " << BASE_DIRECTORY << std::endl;
        
        std::filesystem::path validFile = BASE_DIRECTORY / "test_image.jpg";
        std::filesystem::path largeFile = BASE_DIRECTORY / "large_image.jpg";
        std::filesystem::path outsideFile = std::filesystem::temp_directory_path() / "outside_image.jpg";

        std::ofstream(validFile) << "dummy image data";
        std::ofstream(largeFile, std::ios::binary).seekp(MAX_FILE_SIZE).write("", 1);
        std::ofstream(outsideFile) << "should not be accessed";
        std::cout << "Created test files." << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
}

void cleanupTestEnvironment() {
    try {
        std::filesystem::remove(BASE_DIRECTORY / "test_image.jpg");
        std::filesystem::remove(BASE_DIRECTORY / "large_image.jpg");
        std::filesystem::remove(std::filesystem::temp_directory_path() / "outside_image.jpg");
        std::cout << "\nCleaned up test files." << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error during cleanup: " << e.what() << std::endl;
    }
}

int main() {
    setupTestEnvironment();
    
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid file
    std::cout << "\n1. Testing with a valid file..." << std::endl;
    storeImageInDb((BASE_DIRECTORY / "test_image.jpg").string());

    // Test Case 2: File is too large
    std::cout << "\n2. Testing with a file that is too large..." << std::endl;
    storeImageInDb((BASE_DIRECTORY / "large_image.jpg").string());

    // Test Case 3: Path traversal attempt
    std::cout << "\n3. Testing with a file outside the allowed directory..." << std::endl;
    storeImageInDb((std::filesystem::temp_directory_path() / "outside_image.jpg").string());
    
    // Test Case 4: Non-existent file
    std::cout << "\n4. Testing with a non-existent file..." << std::endl;
    storeImageInDb((BASE_DIRECTORY / "nonexistent.jpg").string());
    
    // Test Case 5: Path is a directory
    std::cout << "\n5. Testing with a path that is a directory..." << std::endl;
    storeImageInDb(BASE_DIRECTORY.string());

    cleanupTestEnvironment();
    
    return 0;
}