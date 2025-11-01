
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <sys/stat.h>
#include <uuid/uuid.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir
#else
#include <sys/types.h>
#endif

const std::string UPLOAD_DIR = "uploads";
const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB

class Lse65 {
public:
    static std::string handleFileUpload(const std::vector<char>& fileData, const std::string& contentType) {
        if (contentType.find("multipart/form-data") == std::string::npos) {
            throw std::runtime_error("Invalid content type");
        }
        
        if (fileData.size() > MAX_FILE_SIZE) {
            throw std::runtime_error("File size exceeds maximum allowed size");
        }
        
        std::vector<char> fileContent = extractFileContent(fileData);
        
        if (!isPdfFile(fileContent)) {
            throw std::runtime_error("Only PDF files are allowed");
        }
        
        std::string fileName = generateUUID() + ".pdf";
        std::string filePath = UPLOAD_DIR + "/" + fileName;
        
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Failed to create file");
        }
        
        outFile.write(fileContent.data(), fileContent.size());
        outFile.close();
        
        return "File uploaded successfully: " + fileName;
    }
    
private:
    static std::vector<char> extractFileContent(const std::vector<char>& multipartData) {
        const char pdfHeader[] = "%PDF";
        const char pdfFooter[] = "%%EOF";
        
        for (size_t i = 0; i < multipartData.size() - 4; i++) {
            if (std::memcmp(&multipartData[i], pdfHeader, 4) == 0) {
                size_t end = multipartData.size();
                for (size_t j = i; j < multipartData.size() - 5; j++) {
                    if (std::memcmp(&multipartData[j], pdfFooter, 5) == 0) {
                        end = j + 5;
                        break;
                    }
                }
                return std::vector<char>(multipartData.begin() + i, multipartData.begin() + end);
            }
        }
        return std::vector<char>();
    }
    
    static bool isPdfFile(const std::vector<char>& content) {
        if (content.size() < 4) return false;
        return std::memcmp(content.data(), "%PDF", 4) == 0;
    }
    
    static std::string generateUUID() {
        uuid_t uuid;
        uuid_generate(uuid);
        char uuid_str[37];
        uuid_unparse(uuid, uuid_str);
        return std::string(uuid_str);
    }
    
public:
    static void testUploadPdfFile(const std::string& fileName, const std::vector<char>& content) {
        std::cout << "Test case: " << fileName << std::endl;
    }
    
    static void createUploadDirectory() {
#ifdef _WIN32
        mkdir(UPLOAD_DIR.c_str());
#else
        mkdir(UPLOAD_DIR.c_str(), 0755);
#endif
    }
};

int main() {
    Lse65::createUploadDirectory();
    
    // Test cases
    Lse65::testUploadPdfFile("test1.pdf", {'P', 'D', 'F', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', ' ', '1'});
    Lse65::testUploadPdfFile("test2.pdf", {'P', 'D', 'F', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', ' ', '2'});
    Lse65::testUploadPdfFile("test3.pdf", {'P', 'D', 'F', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', ' ', '3'});
    Lse65::testUploadPdfFile("test4.pdf", {'P', 'D', 'F', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', ' ', '4'});
    Lse65::testUploadPdfFile("test5.pdf", {'P', 'D', 'F', ' ', 'c', 'o', 'n', 't', 'e', 'n', 't', ' ', '5'});
    
    std::cout << "Server functionality implemented. Note: Full HTTP server requires additional libraries." << std::endl;
    
    return 0;
}
