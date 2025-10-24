
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <sstream>
#include <iomanip>
#include <regex>
#include <ctime>
#include <sys/stat.h>
#include <openssl/sha.h>

class Task50 {
private:
    static const std::string UPLOAD_DIR;
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static std::set<std::string> ALLOWED_EXTENSIONS;
    
    static bool isValidFilename(const std::string& filename) {
        if (filename.empty()) return false;
        
        // Check for path traversal attempts
        if (filename.find("..") != std::string::npos || 
            filename.find("/") != std::string::npos || 
            filename.find("\\\\") != std::string::npos) {\n            return false;\n        }\n        \n        // Check for valid characters\n        std::regex validPattern("^[a-zA-Z0-9._-]+$");\n        return std::regex_match(filename, validPattern);\n    }\n    \n    static std::string getFileExtension(const std::string& filename) {\n        size_t lastDot = filename.find_last_of('.');\n        if (lastDot != std::string::npos && lastDot < filename.length() - 1) {\n            return filename.substr(lastDot + 1);\n        }\n        return "";\n    }\n    \n    static std::string sanitizeFilename(const std::string& filename) {\n        std::string result = filename;\n        std::regex invalidChars("[^a-zA-Z0-9._-]");\n        return std::regex_replace(result, invalidChars, "_");\n    }\n    \n    static std::string generateUniqueFilename(const std::string& filename) {\n        long long timestamp = static_cast<long long>(time(nullptr) * 1000);\n        size_t lastDot = filename.find_last_of('.');\n        std::string name = filename.substr(0, lastDot);\n        std::string extension = getFileExtension(filename);\n        return name + "_" + std::to_string(timestamp) + "." + extension;\n    }\n    \n    static std::string calculateChecksum(const std::vector<unsigned char>& data) {\n        unsigned char hash[SHA256_DIGEST_LENGTH];\n        SHA256(data.data(), data.size(), hash);\n        \n        std::ostringstream oss;\n        for (int i = 0; i < 8; i++) {\n            oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];\n        }\n        return oss.str();\n    }\n    \n    static void createDirectory(const std::string& dir) {\n#ifdef _WIN32\n        _mkdir(dir.c_str());\n#else\n        mkdir(dir.c_str(), 0755);\n#endif\n    }\n    \npublic:\n    static std::string uploadFile(const std::string& filename, const std::vector<unsigned char>& fileContent) {\n        try {\n            // Validate filename\n            if (!isValidFilename(filename)) {\n                return "Error: Invalid filename";\n            }\n            \n            // Check file size\n            if (fileContent.size() > MAX_FILE_SIZE) {\n                return "Error: File size exceeds maximum allowed size (10MB)";\n            }\n            \n            // Check file extension\n            std::string extension = getFileExtension(filename);\n            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);\n            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {\n                return "Error: File type not allowed";\n            }\n            \n            // Create upload directory if it doesn't exist\n            createDirectory(UPLOAD_DIR);\n            \n            // Sanitize filename and create unique name\n            std::string sanitizedName = sanitizeFilename(filename);\n            std::string uniqueFilename = generateUniqueFilename(sanitizedName);\n            std::string filepath = UPLOAD_DIR + "/" + uniqueFilename;\n            \n            // Write file\n            std::ofstream outFile(filepath, std::ios::binary);\n            if (!outFile) {\n                return "Error: Failed to create file";\n            }\n            outFile.write(reinterpret_cast<const char*>(fileContent.data()), fileContent.size());\n            outFile.close();\n            \n            // Calculate checksum\n            std::string checksum = calculateChecksum(fileContent);\n            \n            return "Success: File uploaded successfully. Filename: " + uniqueFilename + \n                   ", Size: " + std::to_string(fileContent.size()) + " bytes, Checksum: " + checksum;\n            \n        } catch (const std::exception& e) {\n            return std::string("Error: Failed to upload file - ") + e.what();\n        }\n    }\n};\n\nconst std::string Task50::UPLOAD_DIR = "uploads";\nstd::set<std::string> Task50::ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};\n\nint main() {\n    std::cout << "File Upload Server - Test Cases\
\
";\n    \n    // Test Case 1: Valid text file\n    std::vector<unsigned char> data1(12);\n    std::string str1 = "Hello World!";\n    std::copy(str1.begin(), str1.end(), data1.begin());\n    std::string result1 = Task50::uploadFile("test1.txt", data1);\n    std::cout << "Test 1: " << result1 << "\
\
";\n    \n    // Test Case 2: Valid PDF file (simulated)\n    std::string str2 = "PDF content simulation";\n    std::vector<unsigned char> data2(str2.begin(), str2.end());\n    std::string result2 = Task50::uploadFile("document.pdf", data2);\n    std::cout << "Test 2: " << result2 << "\
\
";\n    \n    // Test Case 3: Invalid filename with path traversal\n    std::string str3 = "malicious content";\n    std::vector<unsigned char> data3(str3.begin(), str3.end());\n    std::string result3 = Task50::uploadFile("../../../etc/passwd", data3);\n    std::cout << "Test 3: " << result3 << "\
\
";\n    \n    // Test Case 4: Disallowed file extension\n    std::string str4 = "executable content";\n    std::vector<unsigned char> data4(str4.begin(), str4.end());\n    std::string result4 = Task50::uploadFile("script.exe", data4);\n    std::cout << "Test 4: " << result4 << "\
\
";\n    \n    // Test Case 5: Valid image file\n    std::string str5 = "JPEG image data";\n    std::vector<unsigned char> data5(str5.begin(), str5.end());\n    std::string result5 = Task50::uploadFile("photo.jpg", data5);\n    std::cout << "Test 5: " << result5 << "\
\
";
    
    return 0;
}
