
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <sstream>
#include <regex>

namespace fs = std::filesystem;

class Lse70 {
private:
    static const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    static const std::string UPLOAD_DIR;
    static const std::set<std::string> ALLOWED_EXTENSIONS;

    static std::string getFileExtension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos) {
            return filename.substr(pos + 1);
        }
        return "";
    }

    static std::string sanitizeFilename(const std::string& filename) {
        // Extract basename only
        size_t pos = filename.find_last_of("/\\\\");\n        std::string basename = (pos != std::string::npos) ? filename.substr(pos + 1) : filename;\n        \n        // Replace unsafe characters with underscore\n        std::string safe;\n        for (char c : basename) {\n            if (std::isalnum(c) || c == '.' || c == '-' || c == '_') {\n                safe += c;\n            } else {\n                safe += '_';\n            }\n        }\n        return safe;\n    }\n\n    static bool isAllowedExtension(const std::string& filename) {\n        std::string ext = getFileExtension(filename);\n        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);\n        return ALLOWED_EXTENSIONS.find(ext) != ALLOWED_EXTENSIONS.end();\n    }\n\n    static std::string generateUniqueFilename(const std::string& originalFilename) {\n        auto now = std::chrono::system_clock::now();\n        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(\n            now.time_since_epoch()\n        ).count();\n        \n        return std::to_string(timestamp) + "_" + originalFilename;\n    }\n\npublic:\n    static std::string upload_file(const std::string& sourceFile, const std::string& originalFilename) {\n        try {\n            // Create upload directory if it doesn't exist\n            if (!fs::exists(UPLOAD_DIR)) {\n                fs::create_directory(UPLOAD_DIR);\n            }\n\n            // Validate file extension\n            if (!isAllowedExtension(originalFilename)) {\n                return "Error: File type not allowed";\n            }\n\n            // Check if source file exists and get size\n            if (!fs::exists(sourceFile)) {\n                return "Error: Source file not found";\n            }\n\n            size_t fileSize = fs::file_size(sourceFile);\n            if (fileSize > MAX_FILE_SIZE) {\n                return "Error: File size exceeds maximum (5MB)";\n            }\n\n            // Sanitize and generate unique filename\n            std::string safeFilename = sanitizeFilename(originalFilename);\n            std::string uniqueFilename = generateUniqueFilename(safeFilename);\n            std::string destPath = UPLOAD_DIR + "/" + uniqueFilename;\n\n            // Copy file\n            fs::copy_file(sourceFile, destPath, fs::copy_options::overwrite_existing);\n\n            return "Success: File uploaded as " + uniqueFilename;\n\n        } catch (const std::exception& e) {\n            return std::string("Error: ") + e.what();\n        }\n    }\n\n    static void runTests() {\n        std::cout << "File Upload Security Features Implemented:" << std::endl;\n        std::cout << "1. File extension whitelist validation" << std::endl;\n        std::cout << "2. File size limit (5MB)" << std::endl;\n        std::cout << "3. Filename sanitization" << std::endl;\n        std::cout << "4. Unique filename generation" << std::endl;\n        std::cout << "5. Path traversal prevention" << std::endl;\n        std::cout << "\
Test Cases:" << std::endl;\n\n        // Test 1: Valid extension\n        std::cout << "Test 1 - Valid extension 'test.txt': " \n                  << isAllowedExtension("test.txt") << std::endl;\n\n        // Test 2: Invalid extension\n        std::cout << "Test 2 - Invalid extension 'test.exe': " \n                  << isAllowedExtension("test.exe") << std::endl;\n\n        // Test 3: Path traversal attempt\n        std::cout << "Test 3 - Sanitize '../../../etc/passwd': " \n                  << sanitizeFilename("../../../etc/passwd") << std::endl;\n\n        // Test 4: Special characters\n        std::cout << "Test 4 - Sanitize 'test@#$%.txt': " \n                  << sanitizeFilename("test@#$%.txt") << std::endl;\n\n        // Test 5: Unique filename generation\n        std::cout << "Test 5 - Generate unique filename: " \n                  << generateUniqueFilename("test.txt") << std::endl;\n    }\n};\n\nconst std::string Lse70::UPLOAD_DIR = "uploads";\nconst std::set<std::string> Lse70::ALLOWED_EXTENSIONS = {\n    "txt", "pdf", "png", "jpg", "jpeg", "gif", "doc", "docx"
};

int main() {
    Lse70::runTests();
    return 0;
}
