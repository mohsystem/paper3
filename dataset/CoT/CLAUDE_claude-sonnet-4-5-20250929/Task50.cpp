
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <regex>
#include <ctime>
#include <sys/stat.h>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#else
#include <sys/types.h>
#endif

using namespace std;

class Task50 {
private:
    static const string UPLOAD_DIR;
    static const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static const set<string> ALLOWED_EXTENSIONS;

    static string getFileExtension(const string& filename) {
        size_t lastDot = filename.find_last_of('.');
        if (lastDot != string::npos && lastDot < filename.length() - 1) {
            return filename.substr(lastDot + 1);
        }
        return "";
    }

    static string sanitizeFilename(const string& filename) {
        // Extract basename (remove path components)
        size_t lastSlash = filename.find_last_of("/\\\\");\n        string sanitized = (lastSlash != string::npos) ? \n            filename.substr(lastSlash + 1) : filename;\n\n        // Replace unsafe characters\n        regex unsafeChars("[^a-zA-Z0-9._-]");\n        sanitized = regex_replace(sanitized, unsafeChars, "_");\n\n        return sanitized;\n    }\n\n    static string generateUniqueFilename(const string& originalFilename) {\n        string timestamp = to_string(time(nullptr) * 1000);\n        \n        size_t lastDot = originalFilename.find_last_of('.');\n        string nameWithoutExt = (lastDot != string::npos) ? \n            originalFilename.substr(0, lastDot) : originalFilename;\n        string extension = getFileExtension(originalFilename);\n\n        string uniqueName = nameWithoutExt + "_" + timestamp;\n        if (!extension.empty()) {\n            uniqueName += "." + extension;\n        }\n\n        return uniqueName;\n    }\n\n    static bool createDirectory(const string& dir) {\n        struct stat info;\n        if (stat(dir.c_str(), &info) != 0) {\n            return mkdir(dir.c_str(), 0700) == 0;\n        }\n        return (info.st_mode & S_IFDIR) != 0;\n    }\n\npublic:\n    struct UploadResult {\n        bool success;\n        string message;\n        string savedPath;\n\n        UploadResult(bool s, const string& m, const string& p = "") \n            : success(s), message(m), savedPath(p) {}\n    };\n\n    static UploadResult uploadFile(const string& filename, const vector<char>& fileContent) {\n        try {\n            // Validate filename\n            if (filename.empty()) {\n                return UploadResult(false, "Invalid filename");\n            }\n\n            // Sanitize filename\n            string sanitizedFilename = sanitizeFilename(filename);\n\n            // Validate file extension\n            string extension = getFileExtension(sanitizedFilename);\n            transform(extension.begin(), extension.end(), extension.begin(), ::tolower);\n            \n            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {\n                return UploadResult(false, "File type not allowed: " + extension);\n            }\n\n            // Validate file size\n            if (fileContent.empty()) {\n                return UploadResult(false, "Empty file content");\n            }\n            if (fileContent.size() > MAX_FILE_SIZE) {\n                return UploadResult(false, "File size exceeds maximum limit");\n            }\n\n            // Create upload directory\n            if (!createDirectory(UPLOAD_DIR)) {\n                return UploadResult(false, "Failed to create upload directory");\n            }\n\n            // Generate unique filename\n            string uniqueFilename = generateUniqueFilename(sanitizedFilename);\n            string targetPath = UPLOAD_DIR + "/" + uniqueFilename;\n\n            // Write file\n            ofstream outFile(targetPath, ios::binary);\n            if (!outFile) {\n                return UploadResult(false, "Failed to create file");\n            }\n\n            outFile.write(fileContent.data(), fileContent.size());\n            outFile.close();\n\n            // Set file permissions (Unix-like systems)\n#ifndef _WIN32\n            chmod(targetPath.c_str(), S_IRUSR | S_IWUSR);\n#endif\n\n            return UploadResult(true, \n                "File uploaded successfully: " + uniqueFilename, \n                targetPath);\n\n        } catch (const exception& e) {\n            return UploadResult(false, string("Upload failed: ") + e.what());\n        }\n    }\n};\n\nconst string Task50::UPLOAD_DIR = "uploads";\nconst set<string> Task50::ALLOWED_EXTENSIONS = {\n    "txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx"\n};\n\nint main() {\n    cout << "=== Secure File Upload Server - Test Cases ===\
\
";\n\n    // Test Case 1: Valid text file upload\n    string content1 = "This is a test file content.";\n    vector<char> fileContent1(content1.begin(), content1.end());\n    Task50::UploadResult result1 = Task50::uploadFile("test_document.txt", fileContent1);\n    cout << "Test 1 - Valid text file:\
";\n    cout << "Success: " << (result1.success ? "true" : "false") << "\
";\n    cout << "Message: " << result1.message << "\
\
";\n\n    // Test Case 2: Valid PDF file upload\n    vector<char> fileContent2(1024, 'A');\n    Task50::UploadResult result2 = Task50::uploadFile("report.pdf", fileContent2);\n    cout << "Test 2 - Valid PDF file:\
";\n    cout << "Success: " << (result2.success ? "true" : "false") << "\
";\n    cout << "Message: " << result2.message << "\
\
";\n\n    // Test Case 3: Invalid file extension\n    string content3 = "malware";\n    vector<char> fileContent3(content3.begin(), content3.end());\n    Task50::UploadResult result3 = Task50::uploadFile("malicious.exe", fileContent3);\n    cout << "Test 3 - Invalid extension (.exe):\
";\n    cout << "Success: " << (result3.success ? "true" : "false") << "\
";\n    cout << "Message: " << result3.message << "\
\
";\n\n    // Test Case 4: Path traversal attempt\n    string content4 = "hack";\n    vector<char> fileContent4(content4.begin(), content4.end());\n    Task50::UploadResult result4 = Task50::uploadFile("../../etc/passwd.txt", fileContent4);\n    cout << "Test 4 - Path traversal attempt:\
";\n    cout << "Success: " << (result4.success ? "true" : "false") << "\
";\n    cout << "Message: " << result4.message << "\
\
";\n\n    // Test Case 5: Empty file\n    vector<char> fileContent5;\n    Task50::UploadResult result5 = Task50::uploadFile("empty.txt", fileContent5);\n    cout << "Test 5 - Empty file:\
";\n    cout << "Success: " << (result5.success ? "true" : "false") << "\
";\n    cout << "Message: " << result5.message << "\
\
";

    return 0;
}
