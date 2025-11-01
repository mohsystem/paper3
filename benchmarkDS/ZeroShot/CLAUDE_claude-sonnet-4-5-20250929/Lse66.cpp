
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <chrono>
#include <regex>
#include <cstring>

namespace fs = std::filesystem;

class Lse66 {
private:
    static const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    static const std::string UPLOAD_DIR;
    
public:
    static bool isValidPdfExtension(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos == std::string::npos) return false;
        std::string ext = filename.substr(pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == "pdf";
    }
    
    static bool isValidPdfContent(const std::string& filepath) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file) return false;
        
        char header[4];
        file.read(header, 4);
        file.close();
        
        return (header[0] == '%' && header[1] == 'P' && 
                header[2] == 'D' && header[3] == 'F');
    }
    
    static std::string sanitizeFilename(const std::string& filename) {
        std::string sanitized = filename;
        std::regex pattern("[^a-zA-Z0-9._-]");
        return std::regex_replace(sanitized, pattern, "_");
    }
    
    static std::string generateUniqueFilename(const std::string& filename) {
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        size_t pos = filename.find_last_of('.');
        std::string name = filename.substr(0, pos);
        std::string ext = filename.substr(pos);
        
        return name + "_" + std::to_string(timestamp) + ext;
    }
    
    static bool savePdf(const std::string& sourceFile, const std::string& filename) {
        try {
            if (!fs::exists(UPLOAD_DIR)) {
                fs::create_directories(UPLOAD_DIR);
            }
            
            if (!isValidPdfExtension(filename)) {
                std::cerr << "Invalid file extension" << std::endl;
                return false;
            }
            
            size_t fileSize = fs::file_size(sourceFile);
            if (fileSize > MAX_FILE_SIZE) {
                std::cerr << "File too large" << std::endl;
                return false;
            }
            
            std::string sanitized = sanitizeFilename(filename);
            std::string unique = generateUniqueFilename(sanitized);
            std::string destPath = UPLOAD_DIR + "/" + unique;
            
            fs::copy_file(sourceFile, destPath, fs::copy_options::overwrite_existing);
            
            if (!isValidPdfContent(destPath)) {
                fs::remove(destPath);
                std::cerr << "Invalid PDF content" << std::endl;
                return false;
            }
            
            std::cout << "File uploaded successfully: " << unique << std::endl;
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

const std::string Lse66::UPLOAD_DIR = "uploads";

int main() {
    std::cout << "Test Cases for PDF Upload:" << std::endl;
    std::cout << "1. Valid PDF: document.pdf" << std::endl;
    std::cout << "2. Invalid extension: document.txt" << std::endl;
    std::cout << "3. Large file: > 5MB" << std::endl;
    std::cout << "4. Invalid PDF content" << std::endl;
    std::cout << "5. Special characters: doc@#$%.pdf" << std::endl;
    
    return 0;
}
