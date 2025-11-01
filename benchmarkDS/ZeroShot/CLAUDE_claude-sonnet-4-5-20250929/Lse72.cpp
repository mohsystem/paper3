
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <chrono>
#include <sys/stat.h>

class Lse72 {
private:
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static inline std::set<std::string> ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};
    static inline const char* UPLOAD_DIR = "uploads";
    
    static std::string sanitizeFilename(const std::string& filename) {
        if (filename.empty()) {
            return "";
        }
        
        std::string sanitized = filename;
        
        // Remove path traversal attempts
        size_t pos;
        while ((pos = sanitized.find("..")) != std::string::npos) {
            sanitized.erase(pos, 2);
        }
        while ((pos = sanitized.find("/")) != std::string::npos) {
            sanitized.erase(pos, 1);
        }
        while ((pos = sanitized.find("\\\\")) != std::string::npos) {\n            sanitized.erase(pos, 1);\n        }\n        \n        // Allow only alphanumeric, dots, dashes, and underscores\n        std::regex pattern("^[a-zA-Z0-9._-]+$");\n        if (!std::regex_match(sanitized, pattern)) {\n            return "";\n        }\n        \n        return sanitized;\n    }\n    \n    static bool isAllowedExtension(const std::string& filename) {\n        size_t lastDot = filename.find_last_of('.');\n        if (lastDot == std::string::npos) {\n            return false;\n        }\n        \n        std::string extension = filename.substr(lastDot + 1);\n        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);\n        \n        return ALLOWED_EXTENSIONS.find(extension) != ALLOWED_EXTENSIONS.end();\n    }\n    \n    static std::string generateUniqueFilename(const std::string& filename) {\n        auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();\n        \n        size_t lastDot = filename.find_last_of('.');\n        if (lastDot == std::string::npos) {\n            return filename + "_" + std::to_string(timestamp);\n        }\n        \n        std::string name = filename.substr(0, lastDot);\n        std::string extension = filename.substr(lastDot);\n        \n        return name + "_" + std::to_string(timestamp) + extension;\n    }\n    \npublic:\n    static std::string uploadFile(const std::vector<char>& fileContent, const std::string& filename) {\n        try {\n            // Validate file size\n            if (fileContent.size() > MAX_FILE_SIZE) {\n                return "Error: File size exceeds maximum allowed size";\n            }\n            \n            // Sanitize filename\n            std::string sanitizedFilename = sanitizeFilename(filename);\n            if (sanitizedFilename.empty()) {\n                return "Error: Invalid filename";\n            }\n            \n            // Validate file extension\n            if (!isAllowedExtension(sanitizedFilename)) {\n                return "Error: File type not allowed";\n            }\n            \n            // Create uploads directory if it doesn't exist\n            #ifdef _WIN32\n                _mkdir(UPLOAD_DIR);\n            #else\n                mkdir(UPLOAD_DIR, 0755);\n            #endif\n            \n            // Generate unique filename\n            std::string uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            std::string filePath = std::string(UPLOAD_DIR) + "/" + uniqueFilename;\n            \n            // Save file\n            std::ofstream outFile(filePath, std::ios::binary);\n            if (!outFile) {\n                return "Error: Could not create file";\n            }\n            \n            outFile.write(fileContent.data(), fileContent.size());\n            outFile.close();\n            \n            return "File uploaded successfully: " + uniqueFilename;\n        } catch (const std::exception& e) {\n            return "Error: " + std::string(e.what());\n        }\n    }\n};\n\nint main() {\n    // Test Case 1: Valid text file\n    std::string content1 = "This is a test file content";\n    std::vector<char> data1(content1.begin(), content1.end());\n    std::cout << Lse72::uploadFile(data1, "test1.txt") << std::endl;\n    \n    // Test Case 2: Valid PDF file\n    std::vector<char> data2(1024, '\\0');\n    std::cout << Lse72::uploadFile(data2, "document.pdf") << std::endl;\n    \n    // Test Case 3: Invalid extension\n    std::cout << Lse72::uploadFile(data1, "malicious.exe") << std::endl;\n    \n    // Test Case 4: Path traversal attempt\n    std::cout << Lse72::uploadFile(data1, "../../../etc/passwd") << std::endl;\n    \n    // Test Case 5: Valid image file\n    std::vector<char> data5(2048, '\\0');\n    std::cout << Lse72::uploadFile(data5, "photo.jpg") << std::endl;
    
    return 0;
}
