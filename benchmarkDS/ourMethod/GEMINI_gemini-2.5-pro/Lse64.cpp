#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <memory>
#include <random>
#include <chrono>

// MySQL Connector/C++ headers
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/exception.h>

namespace fs = std::filesystem;

const std::string UPLOAD_DIRECTORY = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::string ALLOWED_EXTENSION = ".pdf";

// In a real application, do not hard-code credentials.
// Read from environment variables or a secure config file.
const char* DB_HOST_ENV = std::getenv("DB_HOST");
const char* DB_USER_ENV = std::getenv("DB_USER");
const char* DB_PASS_ENV = std::getenv("DB_PASS");
const char* DB_NAME_ENV = std::getenv("DB_NAME");

/**
 * Uploads a PDF file, saves it securely, and records the filepath in the database.
 *
 * @param file_data The binary content of the file.
 * @param original_filename The original filename from the user.
 * @return The secure filepath where the file was saved. Throws on failure.
 */
std::string upload_pdf(const std::vector<char>& file_data, const std::string& original_filename) {
    // 1. Validate inputs
    if (file_data.empty() || original_filename.empty()) {
        throw std::invalid_argument("Error: Invalid input provided.");
    }

    if (file_data.size() > MAX_FILE_SIZE) {
        throw std::runtime_error("Error: File size exceeds maximum limit.");
    }

    // 2. Sanitize and validate filename
    fs::path original_path(original_filename);
    std::string basename = original_path.filename().string();
    std::string ext = original_path.extension().string();
    
    // Convert extension to lowercase for case-insensitive comparison
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext != ALLOWED_EXTENSION) {
        throw std::runtime_error("Error: Invalid file type. Only " + ALLOWED_EXTENSION + " files allowed.");
    }

    // 3. Generate a secure, unique filename
    auto timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 1000000);
    std::string secure_filename = std::to_string(timestamp) + "_" + std::to_string(distrib(gen)) + ext;

    // 4. Create directory and save the file
    fs::path upload_dir_path(UPLOAD_DIRECTORY);
    if (!fs::exists(upload_dir_path)) {
        if (!fs::create_directories(upload_dir_path)) {
            throw std::runtime_error("Error: Could not create upload directory.");
        }
    }

    fs::path destination_path = upload_dir_path / secure_filename;
    destination_path = fs::absolute(destination_path).lexically_normal();

    // Final security check after normalization
    if (destination_path.string().find(fs::absolute(upload_dir_path).string()) != 0) {
        throw std::runtime_error("Error: Path traversal detected after normalization.");
    }
    
    std::ofstream out_file(destination_path, std::ios::binary);
    if (!out_file) {
        throw std::runtime_error("Error: Could not open file for writing.");
    }
    out_file.write(file_data.data(), file_data.size());
    out_file.close();

    // 5. Save to database
    if (!DB_HOST_ENV || !DB_USER_ENV || !DB_PASS_ENV || !DB_NAME_ENV) {
        std::cerr << "Database environment variables not set. Skipping DB insert." << std::endl;
        std::cout << "File saved at (simulation): " << destination_path.string() << std::endl;
        return destination_path.string();
    }
    
    try {
        sql::Driver* driver = get_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(DB_HOST_ENV, DB_USER_ENV, DB_PASS_ENV));
        con->setSchema(DB_NAME_ENV);

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("INSERT INTO files (filepath) VALUES (?)"));
        pstmt->setString(1, destination_path.string());
        pstmt->executeUpdate();
    } catch (sql::SQLException &e) {
        std::cerr << "Database error: " << e.what() << std::endl;
        // Rollback: Clean up file if DB fails
        fs::remove(destination_path);
        throw std::runtime_error("Database operation failed.");
    }

    std::cout << "File uploaded successfully to: " << destination_path.string() << std::endl;
    return destination_path.string();
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    std::cout << "Note: A running MySQL instance and MySQL Connector/C++ are required." << std::endl;
    std::cout << "Compile with: g++ -std=c++17 your_file.cpp -o app -lmysqlcppconn -lstdc++fs" << std::endl;
    std::cout << "CREATE TABLE files (id INT AUTO_INCREMENT PRIMARY KEY, filepath VARCHAR(255) NOT NULL, created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP);" << std::endl;
    std::cout << "Set DB_HOST, DB_USER, DB_PASS, DB_NAME environment variables." << std::endl;
    
    std::vector<char> pdf_content = {'%', 'P', 'D', 'F', '-', '1', '.', '4'};

    // Test Case 1: Valid PDF
    std::cout << "\n--- Test Case 1: Valid PDF ---" << std::endl;
    try {
        upload_pdf(pdf_content, "document.pdf");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Test Case 2: Invalid file type
    std::cout << "\n--- Test Case 2: Invalid file type ---" << std::endl;
    try {
        upload_pdf(pdf_content, "image.jpg");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Test Case 3: Path traversal attempt
    std::cout << "\n--- Test Case 3: Path traversal attempt ---" << std::endl;
    try {
        upload_pdf(pdf_content, "../../root/.bashrc.pdf");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Test Case 4: File too large
    std::cout << "\n--- Test Case 4: File too large ---" << std::endl;
    try {
        std::vector<char> large_content(MAX_FILE_SIZE + 1, 'a');
        upload_pdf(large_content, "large.pdf");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    // Test Case 5: Empty input
    std::cout << "\n--- Test Case 5: Empty input ---" << std::endl;
    try {
        upload_pdf({}, "");
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    
    return 0;
}