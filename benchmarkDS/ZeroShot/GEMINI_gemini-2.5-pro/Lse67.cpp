#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <stdexcept>

// MySQL Connector/C++ libraries
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

/*
 * Note: To compile and run this code, you need the MySQL Connector/C++.
 * Ensure you have the library installed and linked correctly.
 * Example compilation command:
 * g++ -o Lse67 Lse67.cpp -I/path/to/mysql/connector/include -L/path/to/mysql/connector/lib64 -lmysqlcppconn
 *
 * SQL to create the necessary table:
 * CREATE DATABASE IF NOT EXISTS image_db;
 * USE image_db;
 * CREATE TABLE IF NOT EXISTS images (
 *     id INT AUTO_INCREMENT PRIMARY KEY,
 *     filename VARCHAR(255) NOT NULL,
 *     image_data LONGBLOB NOT NULL,
 *     upload_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP
 * );
 */

class Lse67 {
public:
    static bool storeImageInDB(const std::string& host, const std::string& user,
                               const std::string& password, const std::string& db,
                               const std::string& filePath) {
        
        // 1. Read file into a stream
        std::ifstream fileStream(filePath, std::ios::in | std::ios::binary);
        if (!fileStream) {
            std::cerr << "Error: Cannot open file: " << filePath << std::endl;
            return false;
        }

        try {
            sql::mysql::MySQL_Driver *driver;
            std::unique_ptr<sql::Connection> con;
            std::unique_ptr<sql::PreparedStatement> pstmt;

            driver = sql::mysql::get_mysql_driver_instance();
            con.reset(driver->connect(host, user, password));
            con->setSchema(db);

            // 2. Prepare the statement
            pstmt.reset(con->prepareStatement("INSERT INTO images(filename, image_data) VALUES (?, ?)"));

            // Extract filename from path
            std::string filename = filePath.substr(filePath.find_last_of("/\\") + 1);

            // 3. Bind parameters
            pstmt->setString(1, filename);
            pstmt->setBlob(2, &fileStream);
            
            // 4. Execute the query
            int rowsAffected = pstmt->executeUpdate();
            return rowsAffected > 0;

        } catch (sql::SQLException &e) {
            std::cerr << "Database Error: " << e.what();
            std::cerr << " (MySQL error code: " << e.getErrorCode();
            std::cerr << ", SQLState: " << e.getSQLState() << ")" << std::endl;
            return false;
        } catch (std::exception &e) {
            std::cerr << "Standard Exception: " << e.what() << std::endl;
            return false;
        }
    }

    // Helper to create a dummy file
    static void createDummyFile(const std::string& fileName) {
        std::ofstream outFile(fileName);
        if (outFile.is_open()) {
            outFile << "This is a dummy file.";
            outFile.close();
        }
    }
};

int main() {
    // --- Database Configuration ---
    // IMPORTANT: In a real application, do not hardcode credentials.
    // Use environment variables, a configuration file, or a secrets management service.
    const std::string HOST = "tcp://127.0.0.1:3306";
    const std::string USER = "user"; // Replace with your MySQL username
    const std::string PASS = "password"; // Replace with your MySQL password
    const std::string DB = "image_db";

    // --- Test Cases ---
    std::cout << "Running 5 test cases..." << std::endl;
    std::string dummyFileName = "test_image.png";
    Lse67::createDummyFile(dummyFileName);

    // Test Case 1: Successful Insertion
    std::cout << "\n--- Test Case 1: Successful Insertion ---" << std::endl;
    bool success1 = Lse67::storeImageInDB(HOST, USER, PASS, DB, dummyFileName);
    std::cout << "Result: " << (success1 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 2: File Not Found
    std::cout << "\n--- Test Case 2: File Not Found ---" << std::endl;
    bool success2 = Lse67::storeImageInDB(HOST, USER, PASS, DB, "non_existent_file.jpg");
    std::cout << "Result: " << (success2 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3: Invalid Database Password
    std::cout << "\n--- Test Case 3: Invalid Database Password ---" << std::endl;
    bool success3 = Lse67::storeImageInDB(HOST, USER, "wrong_password", DB, dummyFileName);
    std::cout << "Result: " << (success3 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 4: Path is a Directory (cannot open as a file stream)
    std::cout << "\n--- Test Case 4: Path is a Directory ---" << std::endl;
    bool success4 = Lse67::storeImageInDB(HOST, USER, PASS, DB, "."); // Current directory
    std::cout << "Result: " << (success4 ? "SUCCESS" : "FAILURE") << std::endl;
    
    // Test Case 5: Another successful insertion
    std::cout << "\n--- Test Case 5: Another Successful Insertion ---" << std::endl;
    std::string anotherDummyFile = "another_test.jpg";
    Lse67::createDummyFile(anotherDummyFile);
    bool success5 = Lse67::storeImageInDB(HOST, USER, PASS, DB, anotherDummyFile);
    std::cout << "Result: " << (success5 ? "SUCCESS" : "FAILURE") << std::endl;

    // Cleanup dummy files
    remove(dummyFileName.c_str());
    remove(anotherDummyFile.c_str());

    return 0;
}