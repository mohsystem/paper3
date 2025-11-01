#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>

// Define security constants
const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB
const std::vector<char> PDF_MAGIC_BYTES = {'%', 'P', 'D', 'F', '-'};

/**
 * Securely saves a PDF file to the 'uploads' directory.
 *
 * @param originalFilename The original filename from the user.
 * @param fileContent      The byte content of the file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool savePdf(const std::string& originalFilename, const std::vector<char>& fileContent) {
    // 1. Check for empty inputs
    if (originalFilename.empty() || fileContent.empty()) {
        std::cerr << "Error: Invalid filename or empty file content." << std::endl;
        return false;
    }

    // 2. Security: Check file size
    if (fileContent.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the maximum limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        return false;
    }

    // 3. Security: Validate file type by checking magic bytes
    if (fileContent.size() < PDF_MAGIC_BYTES.size() || 
        !std::equal(PDF_MAGIC_BYTES.begin(), PDF_MAGIC_BYTES.end(), fileContent.begin())) {
        std::cerr << "Error: File is not a valid PDF." << std::endl;
        return false;
    }

    // 4. Security: Prevent path traversal by extracting the base filename
    std::filesystem::path path_obj(originalFilename);
    std::string baseFilename = path_obj.filename().string();

    // 5. Security: Sanitize the filename using a whitelist
    std::string sanitizedFilename;
    for (char c : baseFilename) {
        if (isalnum(c) || c == '.' || c == '_' || c == '-') {
            sanitizedFilename += c;
        } else {
            sanitizedFilename += '_';
        }
    }
    
    // Ensure filename ends with .pdf
    std::string lower_sanitized = sanitizedFilename;
    std::transform(lower_sanitized.begin(), lower_sanitized.end(), lower_sanitized.begin(), ::tolower);
    if (lower_sanitized.rfind(".pdf") != lower_sanitized.length() - 4) {
        sanitizedFilename += ".pdf";
    }

    try {
        // Ensure the upload directory exists
        std::filesystem::path uploadPath(UPLOAD_DIR);
        if (!std::filesystem::exists(uploadPath)) {
            std::filesystem::create_directory(uploadPath);
        }

        // Construct the final, secure destination path
        std::filesystem::path destinationPath = uploadPath / sanitizedFilename;
        
        // Write the file
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: Could not open file for writing: " << destinationPath << std::endl;
            return false;
        }
        
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();

        if (!outFile.good()) {
            std::cerr << "Error: An error occurred while writing to file: " << destinationPath << std::endl;
            return false;
        }

        std::cout << "Successfully saved file to: " << destinationPath << std::endl;
        return true;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        return false;
    }
}

void run_test(const std::string& test_name, const std::string& filename, const std::vector<char>& content) {
    std::cout << "\n[Test " << test_name << "]" << std::endl;
    savePdf(filename, content);
}

int main() {
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    std::vector<char> validPdfContent = {'%', 'P', 'D', 'F', '-', '1', '.', '7', 'c', 'o', 'n', 't', 'e', 'n', 't'};

    // Test Case 1: Valid PDF file
    run_test("1] Valid PDF file", "my_document.pdf", validPdfContent);

    // Test Case 2: Path traversal attempt
    run_test("2] Path traversal attempt", "../../root/.bashrc.pdf", validPdfContent);

    // Test Case 3: Invalid file type (not a PDF)
    std::vector<char> nonPdfContent = {'<', 'h', 't', 'm', 'l', '>'};
    run_test("3] Invalid file type", "index.html", nonPdfContent);

    // Test Case 4: Filename with special characters
    run_test("4] Filename with special characters", "file\\is\"bad?<>!.pdf", validPdfContent);

    // Test Case 5: File too large
    std::vector<char> largeContent(MAX_FILE_SIZE + 1);
    std::copy(PDF_MAGIC_BYTES.begin(), PDF_MAGIC_BYTES.end(), largeContent.begin());
    run_test("5] File too large", "huge_file.pdf", largeContent);

    std::cout << "\n--- C++ Test Cases Finished ---" << std::endl;
    return 0;
}