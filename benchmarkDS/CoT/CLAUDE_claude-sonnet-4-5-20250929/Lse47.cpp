
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <set>

namespace fs = std::filesystem;

class Lse47 {
private:
    static const std::string IMAGES_DIR;
    static const std::set<std::string> ALLOWED_EXTENSIONS;

public:
    static std::vector<char> serveImage(const std::string& filename) {
        try {
            // Sanitize filename - extract only the filename component
            fs::path filePath(filename);
            std::string sanitizedFilename = filePath.filename().string();
            
            // Validate file extension
            std::string extension = filePath.extension().string();
            std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
            
            if (ALLOWED_EXTENSIONS.find(extension) == ALLOWED_EXTENSIONS.end()) {
                return std::vector<char>();
            }
            
            // Construct safe path
            fs::path imagePath = fs::path(IMAGES_DIR) / sanitizedFilename;
            fs::path baseDir = fs::absolute(fs::path(IMAGES_DIR));
            fs::path resolvedPath = fs::absolute(imagePath);
            
            // Ensure the resolved path is within images directory
            auto [rootEnd, nothing] = std::mismatch(baseDir.begin(), baseDir.end(), 
                                                     resolvedPath.begin(), resolvedPath.end());
            if (rootEnd != baseDir.end()) {
                return std::vector<char>();
            }
            
            // Check if file exists and is a regular file
            if (fs::exists(imagePath) && fs::is_regular_file(imagePath)) {
                std::ifstream file(imagePath, std::ios::binary | std::ios::ate);
                if (file.is_open()) {
                    std::streamsize size = file.tellg();
                    file.seekg(0, std::ios::beg);
                    
                    std::vector<char> buffer(size);
                    if (file.read(buffer.data(), size)) {
                        return buffer;
                    }
                }
            }
            
            return std::vector<char>();
        } catch (...) {
            return std::vector<char>();
        }
    }
};

const std::string Lse47::IMAGES_DIR = "images";
const std::set<std::string> Lse47::ALLOWED_EXTENSIONS = {
    ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"
};

int main() {
    // Create images directory if not exists
    fs::create_directories(Lse47::IMAGES_DIR);
    
    // Create test images
    std::vector<std::string> testFiles = {"test1.jpg", "test2.png", "test3.gif", 
                                          "test4.webp", "test5.bmp"};
    for (const auto& testFile : testFiles) {
        fs::path testPath = fs::path(Lse47::IMAGES_DIR) / testFile;
        if (!fs::exists(testPath)) {
            std::ofstream file(testPath);
            file << "Test image content for " << testFile;
        }
    }
    
    // Test cases
    std::cout << "Test 1 - Valid image (test1.jpg): " 
              << (!Lse47::serveImage("test1.jpg").empty() ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "Test 2 - Valid image (test2.png): " 
              << (!Lse47::serveImage("test2.png").empty() ? "SUCCESS" : "FAILED") << std::endl;
    
    std::cout << "Test 3 - Path traversal attempt (../test.jpg): " 
              << (Lse47::serveImage("../test.jpg").empty() ? "SUCCESS (Blocked)" : "FAILED") << std::endl;
    
    std::cout << "Test 4 - Invalid extension (test.txt): " 
              << (Lse47::serveImage("test.txt").empty() ? "SUCCESS (Blocked)" : "FAILED") << std::endl;
    
    std::cout << "Test 5 - Non-existent file (nonexistent.jpg): " 
              << (Lse47::serveImage("nonexistent.jpg").empty() ? "SUCCESS (Not Found)" : "FAILED") << std::endl;
    
    return 0;
}
