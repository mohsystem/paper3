#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <filesystem> // C++17

// MySQL Connector/C++
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

/*
 Pre-requisites:
 1. MySQL server is running.
 2. A database named 'file_db' exists.
 3. A table named 'files' exists in 'file_db'.
    SQL to create table:
    CREATE TABLE files (
        id INT AUTO_INCREMENT PRIMARY KEY,
        filepath VARCHAR(255) NOT NULL,
        uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
    );
 4. MySQL Connector/C++ is installed and linked.
    - Download from the official MySQL website.
    - Include its headers.
    - Link against its libraries (e.g., -lmysqlcppconn)
    - You might need to add the connector's lib directory to your linker path.

 Compilation command example (Linux/g++):
 g++ -std=c++17 -o Lse64 Lse64.cpp -I/path/to/mysql_connector_cpp/include -L/path/to/mysql_connector_cpp/lib64 -lmysqlcppconn
*/

// --- DATABASE CONFIGURATION (Update with your details) ---
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "your_username";
const std::string DB_PASSWORD = "your_password";
const std::string DB_NAME = "file_db";
const std::string UPLOAD_DIR = "uploads";

bool upload_pdf(const std::string& sourceFilePath) {
    namespace fs = std::filesystem;

    fs::path sourcePath(sourceFilePath);

    if (!fs::exists(sourcePath) || !fs::is_regular_file(sourcePath)) {
        std::cerr << "Error: Source file does not exist or is not a regular file: " << sourceFilePath << std::endl;
        return false;
    }

    // Create the uploads directory if it doesn't exist
    fs::path uploadDirPath(UPLOAD_DIR);
    if (!fs::exists(uploadDirPath)) {
        if (!fs::create_directory(uploadDirPath)) {
            std::cerr << "Error: Could not create upload directory: " << UPLOAD_DIR << std::endl;
            return false;
        }
    }

    fs::path destPath = uploadDirPath / sourcePath.filename();
    std::string dbFilePath = destPath.string();
    std::replace(dbFilePath.begin(), dbFilePath.end(), '\\', '/'); // Normalize for DB

    try {
        // 1. Save the file to the uploads folder
        fs::copy(sourcePath, destPath, fs::copy_options::overwrite_existing);
        std::cout << "File saved to: " << destPath << std::endl;

        // 2. Save the filepath to the MySQL database
        std::unique_ptr<sql::Connection> con;
        std::unique_ptr<sql::PreparedStatement> pstmt;
        try {
            sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
            con.reset(driver->connect(DB_HOST, DB_USER, DB_PASSWORD));
            con->setSchema(DB_NAME);

            pstmt.reset(con->prepareStatement("INSERT INTO files (filepath) VALUES (?)"));
            pstmt->setString(1, dbFilePath);
            int updateCount = pstmt->executeUpdate();

            if (updateCount > 0) {
                std::cout << "Filepath saved to database successfully." << std::endl;
                return true;
            } else {
                std::cerr << "Error: Failed to save filepath to database." << std::endl;
                fs::remove(destPath); // Rollback file copy
                return false;
            }
        } catch (sql::SQLException &e) {
            std::cerr << "Database Error: " << e.what() << " (MySQL error code: " << e.getErrorCode() << ")" << std::endl;
            fs::remove(destPath); // Rollback file copy
            return false;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "File I/O Error: " << e.what() << std::endl;
        return false;
    }
    return false; // Should not be reached
}


void createDummyFile(const std::string& path) {
    std::ofstream outfile(path);
    if (outfile.is_open()) {
        outfile << "dummy pdf content";
        outfile.close();
    }
}

int main() {
    namespace fs = std::filesystem;
    // Create a directory for test files
    std::string testDir = "test_files";
    fs::create_directory(testDir);
    
    std::string testFiles[] = {
        "test_files/report-q1.pdf",
        "test_files/invoice-123.pdf",
        "test_files/manual_v2.pdf",
        "test_files/presentation.pdf",
        "test_files/non_existent_file.pdf"
    };
    
    for (int i = 0; i < 4; ++i) {
        createDummyFile(testFiles[i]);
    }
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << ": Uploading " << testFiles[i] << " ---" << std::endl;
        bool result = upload_pdf(testFiles[i]);
        std::cout << "Result: " << (result ? "SUCCESS" : "FAILURE") << std::endl;
    }
    
    return 0;
}