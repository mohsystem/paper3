/*
Prerequisites for C++:
1. MySQL Server running.
2. A database named 'testdb'.
3. A table created with the following SQL command:
   CREATE TABLE images (
       id INT AUTO_INCREMENT PRIMARY KEY,
       name VARCHAR(255) NOT NULL,
       data LONGBLOB NOT NULL
   );
4. MySQL C++ Connector (Connector/C++ 8.0+). You need to install the library
   and link against it during compilation.
   - On Ubuntu/Debian: sudo apt-get install libmysqlcppconn-dev
   - On other systems, download from the official MySQL website.

5. Compilation command (example for g++):
   g++ -std=c++17 your_source_file.cpp -o your_executable -lmysqlcppconn

   NOTE: The classic JDBC-like API is deprecated. This example uses the modern X DevAPI.
   Ensure your MySQL server has the X Plugin enabled (it's on by default in MySQL 8.0+).
*/
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <mysqlx/xdevapi.h>

// --- Database Configuration ---
// WARNING: Storing credentials in code is insecure. Use environment variables
// or a secrets management system in a production environment.
const std::string DB_HOST = "localhost";
const std::string DB_USER = "user"; // Replace with your DB username
const std::string DB_PASSWORD = "password"; // Replace with your DB password
const std::string DB_SCHEMA = "testdb";

/**
 * @brief Stores an image file in a MySQL database using the X DevAPI.
 * @param filePath The path to the image file.
 * @return true if successful, false otherwise.
 */
bool storeImage(const std::string& filePath) {
    try {
        // Read the file into a vector of characters
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open file " << filePath << std::endl;
            return false;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // mysqlx::bytes requires unsigned char
        std::vector<unsigned char> buffer(size);
        if (size > 0 && !file.read(reinterpret_cast<char*>(buffer.data()), size)) {
             std::cerr << "Error: Could not read file " << filePath << std::endl;
             return false;
        }

        // Connect to the database
        mysqlx::Session sess(DB_HOST, 33060, DB_USER, DB_PASSWORD); // 33060 is default X Protocol port
        mysqlx::Schema db = sess.getSchema(DB_SCHEMA);
        mysqlx::Table imagesTable = db.getTable("images");

        // Prepare and execute the insert statement
        // The API handles parameter binding, preventing SQL injection.
        std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
        
        mysqlx::bytes blob_data(buffer.data(), buffer.size());

        auto result = imagesTable.insert("name", "data")
                                 .values(fileName, blob_data)
                                 .execute();
        
        return result.getAffectedItemsCount() > 0;

    } catch (const mysqlx::Error &err) {
        std::cerr << "Database error: " << err << std::endl;
        return false;
    } catch (const std::exception &ex) {
        std::cerr << "Standard exception: " << ex.what() << std::endl;
        return false;
    }
}

// Helper function to create dummy files for testing
void createTestFile(const std::string& path, const std::string& content) {
    std::ofstream ofs(path, std::ios::binary);
    ofs.write(content.c_str(), content.length());
}

// Main function with 5 test cases
int main() {
    // --- Test Setup ---
    std::string dummyImagePath = "test_image.png";
    std::string emptyImagePath = "empty_image.png";
    std::string anotherImagePath = "another_image.jpg";
    std::string longNamePath = "a_very_long_file_name_to_test_database_field_limits_and_handling_of_such_cases_0123456789.bin";

    createTestFile(dummyImagePath, "\x89PNG\x01\x02\x03\x04");
    createTestFile(emptyImagePath, "");
    createTestFile(anotherImagePath, "\x89JPG\x01\x02\x03\x04");
    createTestFile(longNamePath, "long name test");

    std::cout << "Running C++ Image Storage Tests..." << std::endl;

    // Test Case 1: Store a valid image file.
    std::cout << "\n--- Test Case 1: Store a valid image ---" << std::endl;
    bool success1 = storeImage(dummyImagePath);
    std::cout << "Result: " << (success1 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 2: Store another valid image.
    std::cout << "\n--- Test Case 2: Store another valid image ---" << std::endl;
    bool success2 = storeImage(anotherImagePath);
    std::cout << "Result: " << (success2 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3: Try to store a non-existent file.
    std::cout << "\n--- Test Case 3: Store a non-existent file ---" << std::endl;
    bool success3 = storeImage("non_existent_file.jpg");
    std::cout << "Result: " << (success3 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 4: Store a file with a long name.
    std::cout << "\n--- Test Case 4: Store a file with a long name ---" << std::endl;
    bool success4 = storeImage(longNamePath);
    std::cout << "Result: " << (success4 ? "SUCCESS" : "FAILURE") << std::endl;
    
    // Test Case 5: Store an empty file.
    std::cout << "\n--- Test Case 5: Store an empty file ---" << std::endl;
    bool success5 = storeImage(emptyImagePath);
    std::cout << "Result: " << (success5 ? "SUCCESS" : "FAILURE") << std::endl;

    // --- Cleanup ---
    std::cout << "\nCleaning up test files..." << std::endl;
    remove(dummyImagePath.c_str());
    remove(emptyImagePath.c_str());
    remove(anotherImagePath.c_str());
    remove(longNamePath.c_str());
    std::cout << "Done." << std::endl;
    
    return 0;
}