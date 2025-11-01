
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <cstring>
#include <sys/stat.h>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(dir) _mkdir(dir)
#else
#include <sys/types.h>
#define MKDIR(dir) mkdir(dir, 0755)
#endif

class Lse69 {
private:
    static const size_t MAX_FILE_SIZE = 5 * 1024 * 1024; // 5MB
    static constexpr const char* UPLOAD_DIR = "uploads";
    static const std::vector<std::string> ALLOWED_TYPES;
    
    static std::string base64_encode(const std::vector<unsigned char>& data) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        
        std::string ret;
        int i = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        size_t in_len = data.size();
        size_t pos = 0;
        
        while (in_len--) {
            char_array_3[i++] = data[pos++];
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (int j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (int j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while (i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
    static std::string getFileExtension(const std::string& filename) {
        size_t lastDot = filename.find_last_of('.');
        if (lastDot == std::string::npos) {
            return "";
        }
        std::string ext = filename.substr(lastDot);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    
    static bool isAllowedType(const std::string& extension) {
        return std::find(ALLOWED_TYPES.begin(), ALLOWED_TYPES.end(), extension) != ALLOWED_TYPES.end();
    }
    
    static std::string generateUUID() {
        // Simple UUID generation for demonstration
        std::stringstream ss;
        ss << "uuid_" << rand() << "_" << time(nullptr);
        return ss.str();
    }
    
public:
    static std::string uploadImage(const std::vector<unsigned char>& imageData, const std::string& originalFilename) {
        try {
            // Validate file size
            if (imageData.empty() || imageData.size() > MAX_FILE_SIZE) {
                std::cerr << "Invalid file size" << std::endl;
                return "";
            }
            
            // Validate file extension
            std::string extension = getFileExtension(originalFilename);
            if (!isAllowedType(extension)) {
                std::cerr << "File type not allowed" << std::endl;
                return "";
            }
            
            // Create upload directory
            MKDIR(UPLOAD_DIR);
            
            // Generate safe filename
            std::string safeFilename = generateUUID() + "_encoded.txt";
            std::string filePath = std::string(UPLOAD_DIR) + "/" + safeFilename;
            
            // Encode image to base64
            std::string encodedImage = base64_encode(imageData);
            
            // Write to file
            std::ofstream outFile(filePath, std::ios::out | std::ios::trunc);
            if (!outFile) {
                std::cerr << "Error opening file for writing" << std::endl;
                return "";
            }
            
            outFile << encodedImage;
            outFile.close();
            
            return filePath;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return "";
        }
    }
    
    static void runTests() {
        // Test case 1: Valid small image
        std::vector<unsigned char> testImage1(std::begin("test image data 1"), std::end("test image data 1"));
        std::cout << "Test 1: " << uploadImage(testImage1, "test1.jpg") << std::endl;
        
        // Test case 2: Valid PNG image
        std::vector<unsigned char> testImage2(std::begin("test image data 2"), std::end("test image data 2"));
        std::cout << "Test 2: " << uploadImage(testImage2, "test2.png") << std::endl;
        
        // Test case 3: Invalid file type
        std::vector<unsigned char> testImage3(std::begin("test image data 3"), std::end("test image data 3"));
        std::cout << "Test 3: " << uploadImage(testImage3, "test3.exe") << std::endl;
        
        // Test case 4: Empty data
        std::vector<unsigned char> testImage4;
        std::cout << "Test 4: " << uploadImage(testImage4, "test4.jpg") << std::endl;
        
        // Test case 5: Valid GIF image
        std::vector<unsigned char> testImage5(std::begin("test image data 5"), std::end("test image data 5"));
        std::cout << "Test 5: " << uploadImage(testImage5, "test5.gif") << std::endl;
    }
};

const std::vector<std::string> Lse69::ALLOWED_TYPES = {".jpg", ".jpeg", ".png", ".gif"};

int main() {
    srand(time(nullptr));
    Lse69::runTests();
    return 0;
}
