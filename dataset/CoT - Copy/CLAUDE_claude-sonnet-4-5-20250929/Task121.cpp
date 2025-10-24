
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <openssl/sha.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#endif

class Task121 {
private:
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static const std::string UPLOAD_DIR;
    static std::set<std::string> ALLOWED_EXTENSIONS;
    
    static std::string getFileExtension(const std::string& filename) {
        size_t lastDot = filename.find_last_of('.');
        if (lastDot == std::string::npos || lastDot == filename.length() - 1) {
            return "";
        }
        std::string ext = filename.substr(lastDot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    
    static std::string getBasename(const std::string& path) {
        size_t lastSlash = path.find_last_of("/\\\\");\n        if (lastSlash == std::string::npos) {\n            return path;\n        }\n        return path.substr(lastSlash + 1);\n    }\n    \n    static bool isValidFilename(const std::string& filename) {\n        std::regex pattern("^[a-zA-Z0-9_\\\\-\\\\.]+$");\n        return std::regex_match(filename, pattern);\n    }\n    \n    static std::string sha256(const std::string& str) {\n        unsigned char hash[SHA256_DIGEST_LENGTH];\n        SHA256_CTX sha256;\n        SHA256_Init(&sha256);\n        SHA256_Update(&sha256, str.c_str(), str.size());\n        SHA256_Final(hash, &sha256);\n        \n        std::stringstream ss;\n        for(int i = 0; i < 8; i++) {\n            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];\n        }\n        return ss.str();\n    }\n    \n    static std::string generateUniqueFilename(const std::string& originalFilename) {\n        std::string baseName = originalFilename.substr(0, originalFilename.find_last_of('.'));\n        std::string extension = getFileExtension(originalFilename);\n        \n        long long timestamp = std::chrono::system_clock::now().time_since_epoch().count();\n        std::string timestampStr = std::to_string(timestamp);\n        std::string hashStr = sha256(baseName + timestampStr);\n        \n        return baseName + "_" + timestampStr + "_" + hashStr + "." + extension;\n    }\n    \npublic:\n    struct UploadResult {\n        bool success;\n        std::string message;\n        std::string savedPath;\n        \n        UploadResult(bool s, const std::string& m, const std::string& p = "")\n            : success(s), message(m), savedPath(p) {}\n    };\n    \n    static UploadResult uploadFile(const std::string& originalFilename, const std::vector<char>& fileContent) {\n        try {\n            // Validate filename is not empty\n            if (originalFilename.empty()) {\n                return UploadResult(false, "Filename cannot be empty");\n            }\n            \n            // Validate file content is not empty\n            if (fileContent.empty()) {\n                return UploadResult(false, "File content cannot be empty");\n            }\n            \n            // Check file size\n            if (fileContent.size() > MAX_FILE_SIZE) {\n                return UploadResult(false, "File size exceeds maximum limit of 10MB");\n            }\n            \n            // Sanitize filename - remove path traversal attempts\n            std::string sanitizedFilename = getBasename(originalFilename);\n            \n            // Validate filename pattern\n            if (!isValidFilename(sanitizedFilename)) {\n                return UploadResult(false, "Invalid filename. Use only alphanumeric characters, hyphens, underscores, and dots");\n            }\n            \n            // Validate file extension\n            std::string extension = getFileExtension(sanitizedFilename);\n            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {\n                return UploadResult(false, "File type not allowed");\n            }\n            \n            // Create upload directory if it doesn't exist\n            #ifdef _WIN32\n            mkdir(UPLOAD_DIR.c_str());\n            #else\n            mkdir(UPLOAD_DIR.c_str(), 0700);\n            #endif\n            \n            // Generate unique filename\n            std::string uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            std::string uploadPath = UPLOAD_DIR + "/" + uniqueFilename;\n            \n            // Check if file already exists\n            std::ifstream checkFile(uploadPath);\n            if (checkFile.good()) {\n                checkFile.close();\n                return UploadResult(false, "File already exists");\n            }\n            \n            // Write file securely\n            std::ofstream outFile(uploadPath, std::ios::binary | std::ios::trunc);\n            if (!outFile.is_open()) {\n                return UploadResult(false, "Error opening file for writing");\n            }\n            \n            outFile.write(fileContent.data(), fileContent.size());\n            outFile.close();\n            \n            // Set file permissions (owner read/write only)\n            #ifndef _WIN32\n            chmod(uploadPath.c_str(), S_IRUSR | S_IWUSR);\n            #endif\n            \n            return UploadResult(true, "File uploaded successfully", uploadPath);\n            \n        } catch (const std::exception& e) {\n            return UploadResult(false, std::string("Unexpected error: ") + e.what());\n        }\n    }\n};\n\nconst std::string Task121::UPLOAD_DIR = "uploads";\nstd::set<std::string> Task121::ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};\n\nint main() {\n    std::cout << "=== File Upload Program Test Cases ===\
\
";\n    \n    // Test Case 1: Valid text file upload\n    std::cout << "Test 1: Valid text file upload\
";\n    std::string content1 = "Hello, this is a test file content.";\n    std::vector<char> data1(content1.begin(), content1.end());\n    Task121::UploadResult result1 = Task121::uploadFile("test_document.txt", data1);\n    std::cout << "Success: " << (result1.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result1.message << "\
";\n    std::cout << "Path: " << result1.savedPath << "\
\
";\n    \n    // Test Case 2: Invalid file extension\n    std::cout << "Test 2: Invalid file extension (.exe)\
";\n    std::string content2 = "Malicious content";\n    std::vector<char> data2(content2.begin(), content2.end());\n    Task121::UploadResult result2 = Task121::uploadFile("malware.exe", data2);\n    std::cout << "Success: " << (result2.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result2.message << "\
\
";\n    \n    // Test Case 3: Path traversal attempt\n    std::cout << "Test 3: Path traversal attempt\
";\n    std::string content3 = "Attack content";\n    std::vector<char> data3(content3.begin(), content3.end());\n    Task121::UploadResult result3 = Task121::uploadFile("../../etc/passwd.txt", data3);\n    std::cout << "Success: " << (result3.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result3.message << "\
\
";\n    \n    // Test Case 4: Empty filename\n    std::cout << "Test 4: Empty filename\
";\n    std::string content4 = "Some content";\n    std::vector<char> data4(content4.begin(), content4.end());\n    Task121::UploadResult result4 = Task121::uploadFile("", data4);\n    std::cout << "Success: " << (result4.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result4.message << "\
\
";\n    \n    // Test Case 5: File size exceeds limit\n    std::cout << "Test 5: File size exceeds limit\
";\n    std::vector<char> data5(10 * 1024 * 1024 + 1, 'A');\n    Task121::UploadResult result5 = Task121::uploadFile("large_file.txt", data5);\n    std::cout << "Success: " << (result5.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result5.message << "\
\
";
    
    return 0;
}
