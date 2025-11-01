#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <chrono>
#include <random>
#include <sstream>

// For database connectivity. This requires the MySQL Connector/C++.
// The code will not compile without it.
// sudo apt-get install libmysqlcppconn-dev
// To compile: g++ -std=c++17 your_file.cpp -o your_app -lmysqlcppconn
/*
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/exception.h>
*/

// --- Configuration ---
const std::string UPLOAD_DIR = "uploads";
constexpr long long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::vector<unsigned char> PDF_MAGIC_BYTES = {'%', 'P', 'D', 'F', '-'};

// --- Database Configuration ---
// FIXME: Replace with your actual database configuration
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "your_username";
const std::string DB_PASS = "your_password";
const std::string DB_NAME = "your_database";

/**
 * MOCK DATABASE FUNCTION
 * To allow compilation without MySQL Connector/C++, this function simulates the DB operation.
 * Replace its content with the real implementation below.
 */
bool saveFilePathToDb(const std::string& filePath) {
    std::cout << "Attempting to save filepath to DB (MOCKED): " << filePath << std::endl;
    // This is a placeholder for a real table.
    // CREATE TABLE IF NOT EXISTS uploaded_files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);
    // Real implementation would go here.
    std::cout << "Filepath successfully saved to database (MOCKED)." << std::endl;
    return true; // Assume success for the mock
}

/*
 * REAL DATABASE FUNCTION
 * Requires MySQL Connector/C++ to be installed and linked.
 *
bool saveFilePathToDb(const std::string& filePath) {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(DB_HOST, DB_USER, DB_PASS));
        con->setSchema(DB_NAME);

        // Security: Use PreparedStatement to prevent SQL Injection
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO uploaded_files (filepath) VALUES (?)"));
        pstmt->setString(1, filePath);
        pstmt->executeUpdate();
        
        std::cout << "Filepath successfully saved to database." << std::endl;
        return true;
    } catch (sql::SQLException &e) {
        std::cerr << "Error saving filepath to database: " << e.what() << std::endl;
        return false;
    }
}
*/

/**
 * Securely uploads a PDF file, saves it, and records its path in the database.
 *
 * @param fileStream The input stream of the file content.
 * @param originalFilename The original name of the file from the user.
 * @return The secure filepath where the file was saved, or an empty string on failure.
 */
std::string uploadPdf(std::istream& fileStream, const std::string& originalFilename) {
    // 1. Security: Sanitize filename to prevent path traversal
    std::filesystem::path originalPath(originalFilename);
    std::string sanitizedFilename = originalPath.filename().string();
    if (sanitizedFilename.empty()) {
        std::cerr << "Error: Filename is empty or invalid." << std::endl;
        return "";
    }
    
    // 2. Security: Validate file extension
    if (originalPath.extension() != ".pdf") {
        std::cerr << "Error: Invalid file extension. Only .pdf files are allowed." << std::endl;
        return "";
    }
    
    // 3. Security: Validate file content (magic bytes)
    std::vector<char> initial_bytes(PDF_MAGIC_BYTES.size());
    fileStream.read(initial_bytes.data(), initial_bytes.size());
    if (fileStream.gcount() < static_cast<long>(PDF_MAGIC_BYTES.size()) || 
        !std::equal(initial_bytes.begin(), initial_bytes.end(), PDF_MAGIC_BYTES.begin(), 
                    [](char a, unsigned char b) { return static_cast<unsigned char>(a) == b; })) {
        std::cerr << "Error: File is not a valid PDF. Magic bytes mismatch." << std::endl;
        return "";
    }
    fileStream.seekg(0, std::ios::beg); // Rewind stream

    // 4. Security: Generate a unique filename
    auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1000, 9999);
    std::string uniqueFilename = std::to_string(timestamp) + "_" + std::to_string(distrib(gen)) + ".pdf";

    std::filesystem::path destinationPath = std::filesystem::path(UPLOAD_DIR) / uniqueFilename;

    try {
        // Create upload directory if it doesn't exist
        std::filesystem::create_directories(destinationPath.parent_path());
        
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: Could not open destination file for writing." << std::endl;
            return "";
        }
        
        // 5. Save file and check size simultaneously
        char buffer[8192];
        long long totalBytesWritten = 0;
        while (fileStream.read(buffer, sizeof(buffer)) || fileStream.gcount() > 0) {
            totalBytesWritten += fileStream.gcount();
            if (totalBytesWritten > MAX_FILE_SIZE) {
                std::cerr << "Error: File size exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
                outFile.close();
                std::filesystem::remove(destinationPath); // Clean up
                return "";
            }
            outFile.write(buffer, fileStream.gcount());
        }
        outFile.close();
        
        if(totalBytesWritten == 0){
             std::cerr << "Error: Empty file uploaded." << std::endl;
             std::filesystem::remove(destinationPath); // Clean up
             return "";
        }
        
        std::cout << "File saved successfully to: " << destinationPath << std::endl;
        
        // 6. Save path to database securely
        if (!saveFilePathToDb(destinationPath.string())) {
            std::cerr << "Error: Database operation failed. Rolling back file save." << std::endl;
            std::filesystem::remove(destinationPath);
            return "";
        }
        
        return destinationPath.string();
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "File system error: " << e.what() << std::endl;
        return "";
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    
    // Test Case 1: Valid PDF upload
    std::cout << "\n[Test Case 1: Valid PDF]" << std::endl;
    std::stringstream validStream("%PDF-1.7\nSome content here.");
    std::string result1 = uploadPdf(validStream, "mydocument.pdf");
    std::cout << "Result: " << (result1.empty() ? "Failure" : "Success") << std::endl;

    // Test Case 2: Non-PDF file
    std::cout << "\n[Test Case 2: Non-PDF file (invalid magic bytes)]" << std::endl;
    std::stringstream invalidStream("This is not a PDF file.");
    std::string result2 = uploadPdf(invalidStream, "not_a_pdf.pdf");
    std::cout << "Result: " << (result2.empty() ? "Failure" : "Success") << std::endl;

    // Test Case 3: Malicious filename (path traversal attempt)
    std::cout << "\n[Test Case 3: Malicious filename]" << std::endl;
    std::stringstream validStream2("%PDF-1.7\nAnother doc.");
    std::string result3 = uploadPdf(validStream2, "../../etc/passwd.pdf");
    std::cout << "Result: " << (result3.empty() ? "Failure" : "Success") << std::endl;

    // Test Case 4: File too large
    std::cout << "\n[Test Case 4: File too large]" << std::endl;
    std::string largeContent(MAX_FILE_SIZE + 1, 'A');
    largeContent.replace(0, PDF_MAGIC_BYTES.size(), std::string(PDF_MAGIC_BYTES.begin(), PDF_MAGIC_BYTES.end()));
    std::stringstream largeStream(largeContent);
    std::string result4 = uploadPdf(largeStream, "largefile.pdf");
    std::cout << "Result: " << (result4.empty() ? "Failure" : "Success") << std::endl;

    // Test Case 5: Invalid file extension
    std::cout << "\n[Test Case 5: Invalid file extension]" << std::endl;
    std::stringstream validStream3("%PDF-1.7\nDoc with wrong extension.");
    std::string result5 = uploadPdf(validStream3, "document.txt");
    std::cout << "Result: " << (result5.empty() ? "Failure" : "Success") << std::endl;
    
    return 0;
}