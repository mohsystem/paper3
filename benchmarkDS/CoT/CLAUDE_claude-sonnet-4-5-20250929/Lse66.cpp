
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <random>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

const std::string UPLOAD_DIR = "uploads";
const size_t MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB

class Lse66 {
public:
    static std::string uploadPdfFile(const std::vector<char>& fileContent, const std::string& originalFilename) {
        try {
            // Validate file size
            if (fileContent.empty()) {
                return "Error: File is empty";
            }
            if (fileContent.size() > MAX_FILE_SIZE) {
                return "Error: File size exceeds limit";
            }
            
            // Validate filename
            if (originalFilename.empty()) {
                return "Error: Invalid filename";
            }
            
            // Validate PDF extension
            if (!endsWith(originalFilename, ".pdf") && !endsWith(originalFilename, ".PDF")) {
                return "Error: Only PDF files are allowed";
            }
            
            // Validate PDF magic bytes
            if (!isPdfFile(fileContent)) {
                return "Error: File is not a valid PDF";
            }
            
            // Create uploads directory if not exists
            fs::path uploadPath = fs::absolute(UPLOAD_DIR);
            fs::create_directories(uploadPath);
            
            // Generate safe filename
            std::string safeFilename = sanitizeFilename(originalFilename);
            std::string uniqueFilename = generateUuid() + "_" + safeFilename;
            
            // Resolve target path and validate it's within upload directory\n            fs::path targetPath = (uploadPath / uniqueFilename).lexically_normal();\n            if (!startsWith(targetPath.string(), uploadPath.string())) {\n                return "Error: Invalid file path";\n            }\n            \n            // Write file securely\n            std::ofstream outFile(targetPath, std::ios::binary);\n            if (!outFile) {\n                return "Error: Cannot create file";\n            }\n            outFile.write(fileContent.data(), fileContent.size());\n            outFile.close();\n            \n            // Set secure file permissions\n            fs::permissions(targetPath, \n                fs::perms::owner_read | fs::perms::owner_write,\n                fs::perm_options::replace);\n            \n            return "Success: File uploaded as " + uniqueFilename;\n            \n        } catch (const std::exception& e) {\n            return std::string("Error: ") + e.what();\n        }\n    }\n\nprivate:\n    static bool isPdfFile(const std::vector<char>& content) {\n        if (content.size() < 4) return false;\n        return content[0] == 0x25 && content[1] == 0x50 && \n               content[2] == 0x44 && content[3] == 0x46; // %PDF\n    }\n    \n    static std::string sanitizeFilename(const std::string& filename) {\n        std::string sanitized;\n        for (char c : filename) {\n            if (std::isalnum(c) || c == '_' || c == '-' || c == '.') {\n                sanitized += c;\n            } else {\n                sanitized += '_';
            }
        }
        if (sanitized.length() > 100) {
            sanitized = sanitized.substr(0, 100);
        }
        return sanitized;
    }
    
    static std::string generateUuid() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        
        std::stringstream ss;
        ss << std::hex;
        for (int i = 0; i < 16; i++) {
            ss << dis(gen);
        }
        return ss.str();
    }
    
    static bool endsWith(const std::string& str, const std::string& suffix) {
        if (str.length() < suffix.length()) return false;
        return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
    }
    
    static bool startsWith(const std::string& str, const std::string& prefix) {
        return str.find(prefix) == 0;
    }
};

int main() {
    // Test case 1: Valid PDF
    std::vector<char> validPdf = {0x25, 0x50, 0x44, 0x46, 0x2D, 0x31, 0x2E, 0x34};
    std::cout << "Test 1: " << Lse66::uploadPdfFile(validPdf, "test.pdf") << std::endl;
    
    // Test case 2: Invalid extension
    std::cout << "Test 2: " << Lse66::uploadPdfFile(validPdf, "test.txt") << std::endl;
    
    // Test case 3: Empty file
    std::vector<char> emptyFile;
    std::cout << "Test 3: " << Lse66::uploadPdfFile(emptyFile, "empty.pdf") << std::endl;
    
    // Test case 4: Invalid PDF content
    std::vector<char> invalidPdf = {0x00, 0x01, 0x02, 0x03};
    std::cout << "Test 4: " << Lse66::uploadPdfFile(invalidPdf, "fake.pdf") << std::endl;
    
    // Test case 5: Malicious filename
    std::cout << "Test 5: " << Lse66::uploadPdfFile(validPdf, "../../../etc/passwd.pdf") << std::endl;
    
    return 0;
}
