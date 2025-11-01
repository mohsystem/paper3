#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <regex>

// Define a safe location for uploads.
const std::string UPLOADS_DIR_NAME = "uploads";

// Set a reasonable maximum file size (e.g., 10 MB).
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024;

// A strict regex for valid filenames.
const std::regex FILENAME_REGEX("^[a-zA-Z0-9_.-]+$");

/**
 * Checks if a string ends with a given suffix (case-insensitive).
 */
bool endsWithCaseInsensitive(const std::string& mainStr, const std::string& toMatch) {
    if (mainStr.length() < toMatch.length()) {
        return false;
    }
    return std::equal(toMatch.rbegin(), toMatch.rend(), mainStr.rbegin(),
                      [](char a, char b) {
                          return std::tolower(a) == std::tolower(b);
                      });
}

/**
 * Accepts a PDF file's content and saves it securely to an 'uploads' folder.
 *
 * @param fileName The name of the file provided by the user.
 * @param fileContent The byte content of the file.
 * @return true if the file was saved successfully, false otherwise.
 */
bool savePdf(const std::string& fileName, const std::vector<char>& fileContent) {
    // 1. Input validation: size, nulls, and content.
    if (fileName.empty() || fileContent.empty()) {
        std::cerr << "Error: File name or content is empty." << std::endl;
        return false;
    }

    if (fileContent.size() > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the maximum limit of " << MAX_FILE_SIZE << " bytes." << std::endl;
        return false;
    }

    // Basic magic byte check for PDF (%PDF-).
    const std::vector<char> pdfMagicBytes = {'%', 'P', 'D', 'F', '-'};
    if (fileContent.size() < pdfMagicBytes.size() ||
        !std::equal(pdfMagicBytes.begin(), pdfMagicBytes.end(), fileContent.begin())) {
        std::cerr << "Error: File is not a valid PDF." << std::endl;
        return false;
    }

    // 2. Sanitize filename to prevent path traversal and other attacks.
    std::filesystem::path pathObj(fileName);
    std::string sanitizedFileName = pathObj.filename().string();
    
    // Check for null bytes.
    if (sanitizedFileName.find('\0') != std::string::npos) {
        std::cerr << "Error: Invalid filename containing null bytes." << std::endl;
        return false;
    }

    // Validate against a strict whitelist and ensure it ends with .pdf.
    if (!std::regex_match(sanitizedFileName, FILENAME_REGEX) || !endsWithCaseInsensitive(sanitizedFileName, ".pdf")) {
        std::cerr << "Error: Invalid filename. It contains illegal characters or is not a .pdf file." << std::endl;
        return false;
    }

    try {
        // Create the uploads directory if it doesn't exist.
        std::filesystem::path baseDir = std::filesystem::absolute(UPLOADS_DIR_NAME);
        std::filesystem::create_directories(baseDir);
        // Set directory permissions (e.g., rwxr-x---).
        std::filesystem::permissions(baseDir, 
            std::filesystem::perms::owner_all | 
            std::filesystem::perms::group_read | 
            std::filesystem::perms::group_exec,
            std::filesystem::perm_options::replace);

        // 3. Construct the final destination path and verify it's within the upload directory.
        std::filesystem::path destinationPath = (baseDir / sanitizedFileName).lexically_normal();
        
        // Ensure the resolved path is a child of the base directory.
        auto [root_end, nothing] = std::mismatch(baseDir.begin(), baseDir.end(), destinationPath.begin());
        if (root_end != baseDir.end()) {
             std::cerr << "Error: Path traversal attempt detected." << std::endl;
             return false;
        }

        if (std::filesystem::exists(destinationPath)) {
            std::cerr << "Error: File already exists at destination: " << destinationPath << std::endl;
            return false;
        }

        // 4. Secure write: Write to a temporary file and then atomically move it.
        std::filesystem::path tempPath = baseDir / (sanitizedFileName + ".tmp" + std::to_string(rand()));
        
        std::ofstream tempFile(tempPath, std::ios::binary | std::ios::out);
        if (!tempFile.is_open()) {
            std::cerr << "Error: Could not create temporary file." << std::endl;
            return false;
        }

        tempFile.write(fileContent.data(), fileContent.size());
        tempFile.close();
        if (!tempFile) { // Check for write errors
             std::cerr << "Error: Failed to write to temporary file." << std::endl;
             std::filesystem::remove(tempPath);
             return false;
        }

        // Atomically rename the temporary file to the final destination.
        std::filesystem::rename(tempPath, destinationPath);
        std::cout << "Successfully saved file: " << destinationPath << std::endl;
        return true;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "A filesystem error occurred: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Running test cases..." << std::endl;
    
    // Test Case 1: Valid PDF file
    std::cout << "\n--- Test Case 1: Valid PDF ---" << std::endl;
    std::string validPdfStr = "%PDF-1.4\n... a valid pdf content ...";
    std::vector<char> validPdfContent(validPdfStr.begin(), validPdfStr.end());
    savePdf("report.pdf", validPdfContent);

    // Test Case 2: Path traversal attempt
    std::cout << "\n--- Test Case 2: Path Traversal ---" << std::endl;
    savePdf("../../../etc/passwd", validPdfContent);

    // Test Case 3: Non-PDF file
    std::cout << "\n--- Test Case 3: Non-PDF File ---" << std::endl;
    std::string textStr = "This is just a text file.";
    std::vector<char> textContent(textStr.begin(), textStr.end());
    savePdf("not_a_pdf.txt", textContent);

    // Test Case 4: File too large
    std::cout << "\n--- Test Case 4: File Too Large ---" << std::endl;
    std::vector<char> largeContent(MAX_FILE_SIZE + 1, 'a');
    std::copy(validPdfContent.begin(), validPdfContent.end(), largeContent.begin());
    savePdf("large_file.pdf", largeContent);

    // Test Case 5: Invalid filename
    std::cout << "\n--- Test Case 5: Invalid Filename with Null Byte ---" << std::endl;
    savePdf("file_with_null\0.pdf", validPdfContent);

    return 0;
}