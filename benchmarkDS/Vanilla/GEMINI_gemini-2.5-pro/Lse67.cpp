/*
 * Prerequisites for C++:
 * 1. MySQL Server running.
 * 2. A database created. For example: `CREATE DATABASE testdb;`
 * 3. A table for storing images. For example:
 *    CREATE TABLE images (
 *        id INT AUTO_INCREMENT PRIMARY KEY,
 *        name VARCHAR(255) NOT NULL,
 *        data LONGBLOB NOT NULL
 *    );
 * 4. MySQL C++ Connector. You need to install it and link against it.
 *    On Ubuntu: sudo apt-get install libmysqlcppconn-dev
 *    Compilation command:
 *    g++ -o store_image Lse67.cpp -lmysqlcppconn
 *
 * This code uses the legacy C++ connector (mysqlcppconn). For Connector/C++ 8.0,
 * the API is different (X DevAPI). This example sticks to the more traditional JDBC-like API.
 */
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <stdexcept>

// MySQL Connector/C++ headers
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

// --- IMPORTANT: UPDATE DATABASE CONNECTION DETAILS ---
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "your_username";
const std::string DB_PASS = "your_password";
const std::string DB_NAME = "testdb";

/**
 * Stores an image file in the MySQL database.
 * @param filePath The path to the image file.
 * @return true if successful, false otherwise.
 */
bool storeImageInDB(const std::string& filePath) {
    try {
        // Read file into a stream
        std::ifstream fileStream(filePath, std::ios::binary);
        if (!fileStream) {
            std::cerr << "Error: Cannot open file " << filePath << std::endl;
            return false;
        }

        // Get file name from path
        std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);

        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(DB_HOST, DB_USER, DB_PASS));
        con->setSchema(DB_NAME);

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO images (name, data) VALUES (?, ?)"));
        
        pstmt->setString(1, fileName);
        pstmt->setBlob(2, &fileStream);
        
        pstmt->executeUpdate();
        return true;

    } catch (sql::SQLException &e) {
        std::cerr << "SQL Error: " << e.what();
        std::cerr << " (MySQL error code: " << e.getErrorCode();
        std::cerr << ", SQLState: " << e.getSQLState() << ")" << std::endl;
        return false;
    } catch (std::runtime_error &e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        return false;
    }
}

/**
 * Helper function to create a dummy file for testing.
 * @param fileName The name of the file to create.
 */
void createDummyFile(const std::string& fileName) {
    std::ofstream outFile(fileName, std::ios::binary);
    if (outFile) {
        std::string content = "This is dummy content for " + fileName;
        outFile.write(content.c_str(), content.length());
    } else {
        std::cerr << "Could not create dummy file: " << fileName << std::endl;
    }
}

int main() {
    // Test Cases
    const char* testFiles[] = {
        "test_image_1.jpg",
        "test_image_2.png",
        "document.pdf",
        "archive.zip",
        "data.bin"
    };
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < numTests; ++i) {
        const std::string fileName = testFiles[i];
        
        createDummyFile(fileName);

        std::cout << "--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Attempting to store: " << fileName << std::endl;

        if (storeImageInDB(fileName)) {
            std::cout << "Successfully stored '" << fileName << "' in the database." << std::endl;
        } else {
            std::cout << "Failed to store '" << fileName << "' in the database." << std::endl;
        }

        remove(fileName.c_str());
        std::cout << std::endl;
    }

    return 0;
}