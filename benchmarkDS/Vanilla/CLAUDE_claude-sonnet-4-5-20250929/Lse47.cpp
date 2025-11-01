
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Lse47 {
private:
    static const std::string IMAGES_FOLDER;
    
public:
    static std::vector<char> serveImage(const std::string& filename) {
        std::string filePath = IMAGES_FOLDER + "/" + filename;
        std::vector<char> fileContent;
        
        if (checkFileExists(filename)) {
            std::ifstream file(filePath, std::ios::binary | std::ios::ate);
            if (file.is_open()) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                
                fileContent.resize(size);
                if (file.read(fileContent.data(), size)) {
                    return fileContent;
                }
            }
        }
        return fileContent;
    }
    
    static bool checkFileExists(const std::string& filename) {
        std::string filePath = IMAGES_FOLDER + "/" + filename;
        return fs::exists(filePath) && fs::is_regular_file(filePath);
    }
};

const std::string Lse47::IMAGES_FOLDER = "images";

int main() {
    // Create test images folder
    fs::create_directories("images");
    
    // Test cases
    std::vector<std::string> testFiles = {
        "image1.jpg",
        "image2.png",
        "nonexistent.jpg",
        "test.gif",
        "photo.bmp"
    };
    
    std::cout << "Testing file existence and serving:" << std::endl;
    for (const auto& filename : testFiles) {
        bool exists = Lse47::checkFileExists(filename);
        std::cout << "File: " << filename << " - Exists: " << (exists ? "true" : "false") << std::endl;
        
        std::vector<char> content = Lse47::serveImage(filename);
        if (!content.empty()) {
            std::cout << "  -> File served successfully, size: " << content.size() << " bytes" << std::endl;
        } else {
            std::cout << "  -> File not found or error occurred" << std::endl;
        }
    }
    
    return 0;
}
