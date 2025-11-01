
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <regex>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>
#include <openssl/sha.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define UPLOAD_DIR "uploads"

class Lse71 {
private:
    static const std::set<std::string> ALLOWED_EXTENSIONS;

    static std::string sanitizeFilename(const std::string& filename) {
        if (filename.empty()) return "";
        
        // Remove special characters and path separators
        std::string sanitized;
        for (char c : filename) {
            if (std::isalnum(c) || c == '.' || c == '_' || c == '-') {
                sanitized += c;
            } else {
                sanitized += '_';
            }
        }
        
        // Remove path traversal attempts
        size_t pos;
        while ((pos = sanitized.find("..")) != std::string::npos) {
            sanitized.erase(pos, 2);
        }
        
        return sanitized;
    }

    static std::string getFileExtension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos != std::string::npos && pos < filename.length() - 1) {
            return filename.substr(pos + 1);
        }
        return "";
    }

    static std::string sha256(const std::string& input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)input.c_str(), input.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    static std::string generateSecureFilename(const std::string& originalFilename) {
        std::time_t timestamp = std::time(nullptr);
        std::string hashInput = originalFilename + std::to_string(timestamp);
        std::string hashString = sha256(hashInput);
        std::string extension = getFileExtension(originalFilename);
        return hashString + "." + extension;
    }

    static bool createDirectory(const std::string& path) {
        #ifdef _WIN32
            return _mkdir(path.c_str()) == 0 || errno == EEXIST;
        #else
            return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
        #endif
    }

public:
    static std::string handleFileUpload(const std::string& filename, const std::string& fileContent) {
        try {
            // Validate file size
            if (fileContent.length() > MAX_FILE_SIZE) {
                return "Error: File size exceeds limit";
            }

            // Sanitize and validate filename
            std::string sanitizedFilename = sanitizeFilename(filename);
            if (sanitizedFilename.empty()) {
                return "Error: Invalid filename";
            }

            // Validate file extension
            std::string extension = getFileExtension(sanitizedFilename);
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
                return "Error: File type not allowed";
            }

            // Create upload directory if not exists
            createDirectory(UPLOAD_DIR);

            // Generate secure filename
            std::string secureFilename = generateSecureFilename(sanitizedFilename);
            std::string filePath = std::string(UPLOAD_DIR) + "/" + secureFilename;

            // Prevent path traversal
            if (filePath.find("..") != std::string::npos) {
                return "Error: Invalid file path";
            }

            // Write file
            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile) {
                return "Error: Could not write file";
            }
            outFile.write(fileContent.c_str(), fileContent.length());
            outFile.close();

            return "Success: File uploaded as " + secureFilename;

        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }

    static void runTests() {
        std::cout << "=== File Upload Security Test Cases ===\\n\\n";

        // Test Case 1: Valid text file
        std::cout << "Test 1 - Valid txt file: " 
                  << handleFileUpload("document.txt", "Hello, World!") << "\\n";

        // Test Case 2: Valid PDF file
        std::cout << "Test 2 - Valid pdf file: " 
                  << handleFileUpload("report.pdf", "%PDF-1.4") << "\\n";

        // Test Case 3: Invalid file extension
        std::cout << "Test 3 - Invalid extension: " 
                  << handleFileUpload("script.exe", "malicious code") << "\\n";

        // Test Case 4: Path traversal attempt
        std::cout << "Test 4 - Path traversal: " 
                  << handleFileUpload("../../etc/passwd", "test") << "\\n";

        // Test Case 5: Large file
        std::string largeContent(MAX_FILE_SIZE + 1, 'A');
        std::cout << "Test 5 - Oversized file: " 
                  << handleFileUpload("large.txt", largeContent) << "\\n";
    }
};

const std::set<std::string> Lse71::ALLOWED_EXTENSIONS = {
    "txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx"
};

int main() {
    Lse71::runTests();
    return 0;
}
