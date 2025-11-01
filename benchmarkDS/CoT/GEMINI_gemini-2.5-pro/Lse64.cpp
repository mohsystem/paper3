#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <random>
#include <sstream>

namespace fs = std::filesystem;

const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::vector<char> PDF_MAGIC_BYTES = {'%', 'P', 'D', 'F', '-'};

// Forward declaration
void saveFilePathToDatabase(const std::string& filePath);

/**
 * Generates a pseudo-random UUID-like string.
 * NOTE: For production, a proper UUID library would be more robust.
 */
std::string generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 16; ++i) {
        ss << dis(gen);
    }
    return ss.str();
}

/**
 * Simulates uploading a PDF file.
 *
 * @param originalFilename The original filename from the user.
 * @param fileContent The byte content of the file.
 * @return The sanitized filepath where the file was saved, or an empty string on failure.
 */
std::string uploadPdf(const std::string& originalFilename, const std::vector<char>& fileContent) {
    if (originalFilename.empty() || fileContent.empty()) {
        std::cerr << "Error: Invalid input provided." << std::endl;
        return "";
    }

    // 1. Security: Check file size
    if (fileContent.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        return "";
    }
    
    // 2. Security: Validate file type using magic bytes
    if (fileContent.size() < PDF_MAGIC_BYTES.size() || 
        !std::equal(PDF_MAGIC_BYTES.begin(), PDF_MAGIC_BYTES.end(), fileContent.begin())) {
        std::cerr << "Error: Invalid file type. Not a PDF." << std::endl;
        return "";
    }

    // 3. Security: Prevent path traversal by extracting only the filename
    fs::path originalPath(originalFilename);
    std::string baseFilename = originalPath.filename().string();
    
    // Check extension (case-insensitive)
    std::string ext = originalPath.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext != ".pdf") {
        std::cerr << "Error: Filename must end with .pdf." << std::endl;
        return "";
    }

    // 4. Security: Generate a unique filename to prevent overwrites
    std::string uniqueFilename = generateUUID() + ".pdf";
    
    try {
        fs::path uploadPath(UPLOAD_DIR);
        // Create the directory if it doesn't exist
        if (!fs::exists(uploadPath)) {
            fs::create_directories(uploadPath);
        }

        fs::path destinationPath = uploadPath / uniqueFilename;

        // Save the file
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile) {
             std::cerr << "Error: Could not open file for writing: " << destinationPath << std::endl;
             return "";
        }
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();

        std::cout << "File saved successfully to: " << destinationPath.string() << std::endl;

        // 5. Security: Simulate saving the filepath to a database using a prepared statement
        saveFilePathToDatabase(destinationPath.string());

        return destinationPath.string();
    } catch (const fs::filesystem_error& e) {
        std::cerr << "File system error: " << e.what() << std::endl;
        return "";
    }
}


/**
 * Simulates saving a filepath to a MySQL database using parameterized queries to prevent SQL Injection.
 * @param filePath The path to save.
 */
void saveFilePathToDatabase(const std::string& filePath) {
    std::cout << "\n--- Simulating Database Interaction ---" << std::endl;
    // In a real application (e.g., with C++ Connector for MySQL),
    // you would use a prepared statement object.
    std::cout << "SQL (Prepared Statement): INSERT INTO pdf_files (filepath) VALUES (?);" << std::endl;
    std::cout << "Binding parameter 1: " << filePath << std::endl;
    std::cout << "Executing query..." << std::endl;
    std::cout << "Filepath successfully saved in the database." << std::endl;
    std::cout << "-------------------------------------\n" << std::endl;
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid PDF upload
    std::cout << "--- Test Case 1: Valid PDF ---" << std::endl;
    std::vector<char> validPdfContent = {'%', 'P', 'D', 'F', '-', '1', '.', '5'};
    uploadPdf("document.pdf", validPdfContent);

    // Test Case 2: Invalid file type (not a PDF)
    std::cout << "--- Test Case 2: Invalid File Type (TXT) ---" << std::endl;
    std::vector<char> invalidContent = {'T', 'h', 'i', 's'};
    uploadPdf("not_a_pdf.txt", invalidContent);
    
    // Test Case 3: Path traversal attempt
    std::cout << "--- Test Case 3: Path Traversal Attempt ---" << std::endl;
    uploadPdf("../../etc/passwd.pdf", validPdfContent);

    // Test Case 4: File too large
    std::cout << "--- Test Case 4: File Too Large ---" << std::endl;
    std::vector<char> largeContent(MAX_FILE_SIZE + 1, 'A');
    std::copy(validPdfContent.begin(), validPdfContent.end(), largeContent.begin());
    uploadPdf("large_file.pdf", largeContent);

    // Test Case 5: Another valid PDF to show unique naming
    std::cout << "--- Test Case 5: Another Valid PDF ---" << std::endl;
    uploadPdf("report.pdf", validPdfContent);

    return 0;
}