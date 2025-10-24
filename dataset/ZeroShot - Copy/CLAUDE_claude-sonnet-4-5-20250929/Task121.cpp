
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>

const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
const std::string UPLOAD_DIRECTORY = "uploads";

std::set<std::string> ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "doc", "docx"};

struct UploadResult {
    bool success;
    std::string message;
    std::string savedPath;
    
    UploadResult(bool s, const std::string& m, const std::string& p = "")
        : success(s), message(m), savedPath(p) {}
};

std::string getFileExtension(const std::string& filename) {
    size_t lastDot = filename.find_last_of('.');
    if (lastDot != std::string::npos && lastDot < filename.length() - 1) {
        return filename.substr(lastDot + 1);
    }
    return "";
}

std::string getBasename(const std::string& path) {
    size_t lastSlash = path.find_last_of("/\\\\");\n    if (lastSlash != std::string::npos) {\n        return path.substr(lastSlash + 1);\n    }\n    return path;\n}\n\nstd::string sanitizeFilename(const std::string& filename) {\n    std::string baseName = getBasename(filename);\n    std::regex safePattern("^[a-zA-Z0-9_\\\\-\\\\.]+$");\n    \n    if (!std::regex_match(baseName, safePattern)) {\n        return "";\n    }\n    \n    return baseName;\n}\n\nstd::string generateUniqueFilename(const std::string& originalFilename) {\n    time_t now = time(0);\n    std::string timestamp = std::to_string(now);\n    \n    size_t lastDot = originalFilename.find_last_of('.');\n    std::string nameWithoutExt = originalFilename.substr(0, lastDot);\n    std::string extension = getFileExtension(originalFilename);\n    \n    return nameWithoutExt + "_" + timestamp + "." + extension;\n}\n\nbool createDirectory(const std::string& path) {\n#ifdef _WIN32\n    return _mkdir(path.c_str()) == 0 || errno == EEXIST;\n#else\n    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;\n#endif\n}\n\nstd::string toLowerCase(const std::string& str) {\n    std::string result = str;\n    for (char& c : result) {\n        c = std::tolower(c);\n    }\n    return result;\n}\n\nUploadResult uploadFile(const std::string& filename, const std::vector<unsigned char>& fileContent) {\n    // Validate filename\n    if (filename.empty()) {\n        return UploadResult(false, "Invalid filename");\n    }\n    \n    // Sanitize filename\n    std::string sanitizedFilename = sanitizeFilename(filename);\n    if (sanitizedFilename.empty()) {\n        return UploadResult(false, "Filename contains invalid characters");\n    }\n    \n    // Check file extension\n    std::string extension = getFileExtension(sanitizedFilename);\n    if (ALLOWED_EXTENSIONS.find(toLowerCase(extension)) == ALLOWED_EXTENSIONS.end()) {\n        return UploadResult(false, "File type not allowed");\n    }\n    \n    // Validate file size\n    if (fileContent.empty()) {\n        return UploadResult(false, "Empty file");\n    }\n    if (fileContent.size() > MAX_FILE_SIZE) {\n        return UploadResult(false, "File size exceeds maximum limit");\n    }\n    \n    // Create upload directory\n    if (!createDirectory(UPLOAD_DIRECTORY)) {\n        // Directory might already exist, which is fine\n    }\n    \n    // Generate unique filename\n    std::string uniqueFilename = generateUniqueFilename(sanitizedFilename);\n    std::string filePath = UPLOAD_DIRECTORY + "/" + uniqueFilename;\n    \n    // Write file\n    std::ofstream outFile(filePath, std::ios::binary);\n    if (!outFile) {\n        return UploadResult(false, "Error creating file");\n    }\n    \n    outFile.write(reinterpret_cast<const char*>(fileContent.data()), fileContent.size());\n    outFile.close();\n    \n    if (!outFile.good()) {\n        return UploadResult(false, "Error writing file");\n    }\n    \n    return UploadResult(true, "File uploaded successfully", filePath);\n}\n\nint main() {\n    std::cout << "=== File Upload Program - Test Cases ===\
\
";\n    \n    // Test Case 1: Valid text file\n    std::cout << "Test Case 1: Valid text file\
";\n    std::string content1 = "Hello, this is a test file.";\n    std::vector<unsigned char> data1(content1.begin(), content1.end());\n    UploadResult result1 = uploadFile("test.txt", data1);\n    std::cout << "Success: " << (result1.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result1.message << "\
";\n    std::cout << "Path: " << result1.savedPath << "\
\
";\n    \n    // Test Case 2: Valid PDF file\n    std::cout << "Test Case 2: Valid PDF file\
";\n    std::vector<unsigned char> data2(1024, 0x50);\n    UploadResult result2 = uploadFile("document.pdf", data2);\n    std::cout << "Success: " << (result2.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result2.message << "\
";\n    std::cout << "Path: " << result2.savedPath << "\
\
";\n    \n    // Test Case 3: Invalid file extension\n    std::cout << "Test Case 3: Invalid file extension\
";\n    std::string content3 = "Executable content";\n    std::vector<unsigned char> data3(content3.begin(), content3.end());\n    UploadResult result3 = uploadFile("malicious.exe", data3);\n    std::cout << "Success: " << (result3.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result3.message << "\
\
";\n    \n    // Test Case 4: File too large\n    std::cout << "Test Case 4: File too large\
";\n    std::vector<unsigned char> data4(MAX_FILE_SIZE + 1, 0);\n    UploadResult result4 = uploadFile("large_file.txt", data4);\n    std::cout << "Success: " << (result4.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result4.message << "\
\
";\n    \n    // Test Case 5: Path traversal attempt\n    std::cout << "Test Case 5: Path traversal attempt\
";\n    std::string content5 = "Malicious content";\n    std::vector<unsigned char> data5(content5.begin(), content5.end());\n    UploadResult result5 = uploadFile("../../etc/passwd.txt", data5);\n    std::cout << "Success: " << (result5.success ? "true" : "false") << "\
";\n    std::cout << "Message: " << result5.message << "\
\
";
    
    return 0;
}
