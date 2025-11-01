
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <filesystem>
#include <ctime>
#include <random>
#include <algorithm>

namespace fs = std::filesystem;

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const std::set<std::string> ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "gif"};
const std::string UPLOAD_DIR = "uploads";

std::string sanitizeFilename(const std::string& filename) {
    std::regex pattern("[^a-zA-Z0-9._-]");
    return std::regex_replace(filename, pattern, "_");
}

std::string getFileExtension(const std::string& filename) {
    size_t pos = filename.find_last_of('.');
    if (pos != std::string::npos && pos > 0) {
        return filename.substr(pos + 1);
    }
    return "";
}

std::string generateUniqueFilename(const std::string& originalName) {
    std::string extension = getFileExtension(originalName);
    std::string nameWithoutExt = originalName.substr(0, originalName.find_last_of('.'));
    
    auto timestamp = std::time(nullptr);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 9999);
    
    return nameWithoutExt + "_" + std::to_string(timestamp) + 
           "_" + std::to_string(dis(gen)) + "." + extension;
}

std::string upload_file(const std::vector<unsigned char>& fileData, const std::string& originalFilename) {
    try {
        // Create uploads directory if it doesn't exist\n        fs::path uploadPath(UPLOAD_DIR);\n        if (!fs::exists(uploadPath)) {\n            fs::create_directories(uploadPath);\n        }\n        \n        // Validate file size\n        if (fileData.size() > MAX_FILE_SIZE) {\n            return "Error: File size exceeds limit";\n        }\n        \n        // Sanitize and validate filename\n        std::string sanitizedName = sanitizeFilename(originalFilename);\n        if (sanitizedName.empty()) {\n            return "Error: Invalid filename";\n        }\n        \n        // Validate file extension\n        std::string extension = getFileExtension(sanitizedName);\n        std::string lowerExt = extension;\n        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);\n        \n        if (ALLOWED_EXTENSIONS.find(lowerExt) == ALLOWED_EXTENSIONS.end()) {\n            return "Error: File type not allowed";\n        }\n        \n        // Generate unique filename\n        std::string uniqueFilename = generateUniqueFilename(sanitizedName);\n        fs::path filePath = uploadPath / uniqueFilename;\n        \n        // Prevent path traversal\n        fs::path normalizedFilePath = fs::absolute(filePath).lexically_normal();\n        fs::path normalizedUploadPath = fs::absolute(uploadPath).lexically_normal();\n        \n        std::string filePathStr = normalizedFilePath.string();\n        std::string uploadPathStr = normalizedUploadPath.string();\n        \n        if (filePathStr.substr(0, uploadPathStr.length()) != uploadPathStr) {\n            return "Error: Invalid file path";\n        }\n        \n        // Save file\n        std::ofstream outFile(filePath, std::ios::binary);\n        if (!outFile) {\n            return "Error: Could not create file";\n        }\n        outFile.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());\n        outFile.close();\n        \n        return "Success: File uploaded as " + uniqueFilename;\n        \n    } catch (const std::exception& e) {\n        return std::string("Error: ") + e.what();\n    }\n}\n\nint main() {\n    std::cout << "=== File Upload Test Cases ===" << std::endl << std::endl;\n    \n    // Test case 1: Valid text file\n    std::vector<unsigned char> testFile1 = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!'};\n    std::cout << "Test 1 - Valid text file: " << upload_file(testFile1, "test.txt") << std::endl;\n    \n    // Test case 2: Valid image file\n    std::vector<unsigned char> testFile2(1024, 0);\n    std::cout << "Test 2 - Valid image file: " << upload_file(testFile2, "image.jpg") << std::endl;\n    \n    // Test case 3: Invalid extension\n    std::vector<unsigned char> testFile3 = {'s', 'c', 'r', 'i', 'p', 't'};\n    std::cout << "Test 3 - Invalid extension: " << upload_file(testFile3, "malicious.exe") << std::endl;\n    \n    // Test case 4: Path traversal attempt\n    std::vector<unsigned char> testFile4 = {'d', 'a', 't', 'a'};\n    std::cout << "Test 4 - Path traversal attempt: " << upload_file(testFile4, "../../../etc/passwd.txt") << std::endl;\n    \n    // Test case 5: Empty filename\n    std::vector<unsigned char> testFile5 = {'c', 'o', 'n', 't', 'e', 'n', 't'};
    std::cout << "Test 5 - Empty filename: " << upload_file(testFile5, "") << std::endl;
    
    return 0;
}
