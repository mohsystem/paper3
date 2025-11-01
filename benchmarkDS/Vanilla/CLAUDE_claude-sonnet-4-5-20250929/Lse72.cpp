
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <cstring>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

class Lse72 {
private:
    std::string uploadFolder;
    
public:
    Lse72() : uploadFolder("uploads") {
        createUploadDirectory();
    }
    
    void createUploadDirectory() {
        struct stat st;
        if (stat(uploadFolder.c_str(), &st) != 0) {
            mkdir(uploadFolder.c_str(), 0755);
        }
    }
    
    bool uploadFile(const char* data, size_t dataSize, const std::string& filename) {
        try {
            std::string filepath = uploadFolder + "/" + filename;
            std::ofstream outFile(filepath, std::ios::binary);
            
            if (!outFile.is_open()) {
                std::cerr << "Error: Could not open file for writing" << std::endl;
                return false;
            }
            
            outFile.write(data, dataSize);
            outFile.close();
            
            std::cout << "File uploaded successfully: " << filename << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Error uploading file: " << e.what() << std::endl;
            return false;
        }
    }
    
    std::string readFile(const std::string& filename) {
        std::string filepath = uploadFolder + "/" + filename;
        std::ifstream inFile(filepath, std::ios::binary);
        
        if (!inFile.is_open()) {
            return "Error: Could not open file";
        }
        
        std::string content((std::istreambuf_iterator<char>(inFile)),
                           std::istreambuf_iterator<char>());
        inFile.close();
        return content;
    }
};

int main() {
    Lse72 uploader;
    
    // Test Case 1: Upload directory creation
    std::cout << "Test Case 1: Upload directory created" << std::endl;
    struct stat st;
    std::cout << "Directory exists: " << (stat("uploads", &st) == 0 ? "Yes" : "No") << std::endl;
    
    // Test Case 2: Upload a text file
    std::cout << "\\nTest Case 2: Upload text file" << std::endl;
    const char* content1 = "This is test content 1";
    uploader.uploadFile(content1, strlen(content1), "test1.txt");
    
    // Test Case 3: Upload another file
    std::cout << "\\nTest Case 3: Upload another file" << std::endl;
    const char* content2 = "This is test content 2";
    uploader.uploadFile(content2, strlen(content2), "test2.txt");
    
    // Test Case 4: Upload binary data
    std::cout << "\\nTest Case 4: Upload binary data" << std::endl;
    const char binary_data[] = {0x48, 0x65, 0x6C, 0x6C, 0x6F};
    uploader.uploadFile(binary_data, sizeof(binary_data), "binary.dat");
    
    // Test Case 5: Verify file content
    std::cout << "\\nTest Case 5: Verify file content" << std::endl;
    std::string fileContent = uploader.readFile("test1.txt");
    std::cout << "Content of test1.txt: " << fileContent << std::endl;
    
    return 0;
}
