#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <random>
#include <sstream>

// C++17 <filesystem> is required. Compile with: g++ -std=c++17 your_file.cpp -o your_app
namespace fs = std::filesystem;

// --- Configuration ---
const long long MAX_FILE_SIZE_BYTES = 10 * 1024 * 1024; // 10 MB
const std::vector<std::string> ALLOWED_EXTENSIONS = {".txt", ".jpg", ".png", ".pdf", ".docx"};
const std::string UPLOAD_DIRECTORY = "cpp_uploads";

/**
 * Generates a UUID-like random string.
 * @return A random string.
 */
std::string generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static const char* hex_chars = "0123456789abcdef";

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << "-";
        }
        ss << hex_chars[dis(gen)];
    }
    return ss.str();
}

/**
 * Securely "uploads" a file by saving it to a designated directory after security checks.
 *
 * @param originalFileName The name of the file provided by the user.
 * @param fileContent The byte content of the file.
 * @return A confirmation or error message.
 */
std::string uploadFile(const std::string& originalFileName, const std::vector<char>& fileContent) {
    // 1. Check for empty inputs
    if (originalFileName.empty() || fileContent.empty()) {
        return "Error: Invalid file name or content.";
    }

    // 2. Check file size
    if (fileContent.size() > MAX_FILE_SIZE_BYTES) {
        return "Error: File size exceeds the limit of " + std::to_string(MAX_FILE_SIZE_BYTES / (1024 * 1024)) + " MB.";
    }

    // 3. Sanitize filename to prevent path traversal
    fs::path originalPath(originalFileName);
    std::string basename = originalPath.filename().string();
    if (basename.empty() || basename == "." || basename == "..") {
        return "Error: Invalid filename.";
    }

    // 4. Validate file extension against a whitelist
    std::string extension = originalPath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    bool isAllowed = false;
    for (const auto& allowedExt : ALLOWED_EXTENSIONS) {
        if (extension == allowedExt) {
            isAllowed = true;
            break;
        }
    }
    if (!isAllowed) {
        return "Error: File type is not allowed.";
    }

    // 5. Generate a unique, secure filename
    std::string uniqueFileName = generateUUID() + "_" + basename;

    try {
        // 6. Create upload directory if it doesn't exist
        fs::path uploadPath(UPLOAD_DIRECTORY);
        if (!fs::exists(uploadPath)) {
            fs::create_directories(uploadPath);
        }

        // 7. Define the destination path and ensure it's within the upload directory
        fs::path destinationPath = uploadPath / uniqueFileName;
        
        fs::path canonicalUploadPath = fs::canonical(uploadPath);
        fs::path canonicalDestParentPath = fs::canonical(destinationPath.parent_path());

        if (canonicalDestParentPath != canonicalUploadPath) {
             return "Error: Directory traversal attempt detected.";
        }

        // 8. Write the file content
        std::ofstream outFile(destinationPath, std::ios::binary);
        if (!outFile) {
            return "Error: Could not open file for writing.";
        }
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();

        return "Success: File '" + basename + "' uploaded as '" + uniqueFileName + "'.";
    } catch (const fs::filesystem_error& e) {
        // Log the exception server-side
        std::cerr << "File upload failed: " << e.what() << std::endl;
        return "Error: Could not save the file due to a filesystem error.";
    }
}

void runTest(int testNum, const std::string& testName, const std::string& filename, const std::vector<char>& content) {
    std::string result = uploadFile(filename, content);
    std::cout << "Test " << testNum << " (" << testName << "): " << result << std::endl;
}

int main() {
    std::cout << "--- Running C++ File Upload Tests ---" << std::endl;
    if (fs::exists(UPLOAD_DIRECTORY)) {
        fs::remove_all(UPLOAD_DIRECTORY);
    }

    // Test Case 1: Successful upload of a valid text file
    std::string content1_str = "This is a simple text file.";
    std::vector<char> content1(content1_str.begin(), content1_str.end());
    runTest(1, "Valid .txt", "mydocument.txt", content1);

    // Test Case 2: Successful upload of a valid image file (simulated content)
    std::vector<char> content2 = {'\x89', 'P', 'N', 'G', '\r', '\n', '\x1a', '\n'};
    runTest(2, "Valid .png", "photo.png", content2);

    // Test Case 3: Rejected upload due to disallowed file type
    std::string content3_str = "#!/bin/bash\necho 'pwned'";
    std::vector<char> content3(content3_str.begin(), content3_str.end());
    runTest(3, "Invalid extension .sh", "malicious_script.sh", content3);

    // Test Case 4: Rejected upload due to path traversal attempt in filename
    std::vector<char> content4 = {'s', 'o', 'm', 'e', ' ', 'd', 'a', 't', 'a'};
    runTest(4, "Path Traversal", "../../etc/passwd", content4);

    // Test Case 5: Rejected upload due to file size limit
    std::vector<char> content5(MAX_FILE_SIZE_BYTES + 1, '\0');
    runTest(5, "File too large", "largefile.docx", content5);
    
    // Clean up
    if (fs::exists(UPLOAD_DIRECTORY)) {
        fs::remove_all(UPLOAD_DIRECTORY);
    }
    std::cout << "--------------------------------------\n" << std::endl;

    return 0;
}