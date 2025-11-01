#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <algorithm> // For std::transform
#include <cctype>    // For ::tolower

// Use the C++17 filesystem library
namespace fs = std::filesystem;

/**
 * @brief Simulates saving an uploaded PDF file to an 'uploads' directory.
 * @param sourceFilePath The path to the source PDF file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool savePdf(const std::string& sourceFilePath) {
    if (sourceFilePath.empty()) {
        std::cerr << "Error: Source file path is empty." << std::endl;
        return false;
    }

    fs::path sourcePath(sourceFilePath);

    // 1. Check if source file exists and is a regular file
    if (!fs::is_regular_file(sourcePath)) {
        std::cerr << "Error: Source file does not exist or is not a regular file: " << sourceFilePath << std::endl;
        return false;
    }

    // 2. Check if the file is a PDF (case-insensitive)
    if (sourcePath.has_extension()) {
        std::string ext = sourcePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ext != ".pdf") {
            std::cerr << "Error: File is not a PDF: " << sourceFilePath << std::endl;
            return false;
        }
    } else {
        std::cerr << "Error: File has no extension: " << sourceFilePath << std::endl;
        return false;
    }

    // 3. Create the 'uploads' directory if it doesn't exist
    std::string uploadDirName = "uploads";
    fs::path uploadDirPath(uploadDirName);
    std::error_code ec;
    fs::create_directories(uploadDirPath, ec);
    if (ec) {
        std::cerr << "Error: Could not create directory '" << uploadDirName << "'. " << ec.message() << std::endl;
        return false;
    }

    // 4. Copy the file to the uploads directory
    try {
        fs::path destinationPath = uploadDirPath / sourcePath.filename();
        fs::copy(sourcePath, destinationPath, fs::copy_options::overwrite_existing);
        std::cout << "Successfully saved '" << sourceFilePath << "' to '" << destinationPath.string() << "'" << std::endl;
        return true;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Could not save file '" << sourceFilePath << "'. " << e.what() << std::endl;
        return false;
    }
}

// Helper function to create dummy files for testing
void createDummyFile(const std::string& fileName, const std::string& content) {
    std::ofstream outFile(fileName);
    if (outFile.is_open()) {
        outFile << content;
        outFile.close();
    } else {
        std::cerr << "Failed to create dummy file: " << fileName << std::endl;
    }
}

int main() {
    // Setup test environment by creating some dummy files
    std::cout << "--- Setting up test environment ---" << std::endl;
    createDummyFile("test1.pdf", "This is the content of PDF 1.");
    createDummyFile("test2.pdf", "This is the content of PDF 2.");
    createDummyFile("test3.txt", "This is a text file, not a PDF.");
    createDummyFile("test4.pdf", "This is PDF 4, used to test when the uploads dir already exists.");

    std::vector<std::string> testCases = {
        "test1.pdf",          // Case 1: Valid PDF file
        "test2.pdf",          // Case 2: Another valid PDF file
        "test3.txt",          // Case 3: Invalid file type (not a PDF)
        "non_existent.pdf",   // Case 4: File that does not exist
        "test4.pdf"           // Case 5: Valid PDF, where the 'uploads' directory already exists
    };

    std::cout << "\n--- Running Test Cases ---" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "\nTest Case " << i + 1 << ": Processing '" << testCases[i] << "'" << std::endl;
        bool success = savePdf(testCases[i]);
        std::cout << "Result: " << (success ? "SUCCESS" : "FAILURE") << std::endl;
    }

    return 0;
}