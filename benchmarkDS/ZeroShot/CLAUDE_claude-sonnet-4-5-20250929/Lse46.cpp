
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <filesystem>

class Lse46 {
public:
    static std::string sanitizeFilename(const std::string& filename) {
        std::string sanitized;
        for (char c : filename) {
            if (std::isalnum(c) || c == '.' || c == '_' || c == '-') {
                sanitized += c;
            } else {
                sanitized += '_';
            }
        }
        return sanitized;
    }
    
    static std::string img(const std::string& name) {
        // Sanitize filename
        std::string sanitizedName = sanitizeFilename(name);
        
        // Construct secure file path
        std::filesystem::path basePath = std::filesystem::absolute("images");
        std::filesystem::path filePath = basePath / sanitizedName;
        filePath = std::filesystem::canonical(filePath.parent_path()) / filePath.filename();
        
        // Verify file is within images directory
        if (filePath.string().find(basePath.string()) != 0) {
            return "Error: Forbidden access";
        }
        
        // Check if file exists
        if (!std::filesystem::exists(filePath) || !std::filesystem::is_regular_file(filePath)) {
            return "Error: File not found";
        }
        
        // Read and return file content
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            return "Error: Cannot open file";
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    static void createTestFiles() {
        std::filesystem::create_directories("images");
        
        std::string testFiles[] = {"test1.jpg", "test2.png", "test3.gif", "test4.jpeg", "test5.jpg"};
        
        for (const auto& filename : testFiles) {
            std::string filepath = "images/" + filename;
            std::ofstream file(filepath);
            if (file) {
                file << "Test content for " << filename;
                file.close();
            }
        }
    }
};

int main() {
    Lse46::createTestFiles();
    
    std::string testCases[] = {"test1.jpg", "test2.png", "test3.gif", "test4.jpeg", "test5.jpg"};
    
    std::cout << "Running 5 test cases:\\n\\n";
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test case " << (i + 1) << ": " << testCases[i] << std::endl;
        std::string result = Lse46::img(testCases[i]);
        std::cout << "Result: " << result << std::endl << std::endl;
    }
    
    return 0;
}
