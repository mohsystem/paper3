
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

class Lse71 {
private:
    static const long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10MB
    static const std::string UPLOAD_DIR;
    static std::set<std::string> ALLOWED_EXTENSIONS;

public:
    static bool isValidFile(const std::string& filename) {
        size_t pos = filename.find_last_of('.');
        if (pos == std::string::npos) return false;
        
        std::string ext = filename.substr(pos + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        return ALLOWED_EXTENSIONS.find(ext) != ALLOWED_EXTENSIONS.end();
    }

    static std::string sanitizeFilename(const std::string& filename) {
        std::string sanitized = filename;
        std::regex pattern("[^a-zA-Z0-9._-]");
        return std::regex_replace(sanitized, pattern, "_");
    }

    static std::string generateUUID() {
        char uuid[37];
        snprintf(uuid, sizeof(uuid), "%08x-%04x-%04x-%04x-%012x",
                 rand(), rand() & 0xFFFF, rand() & 0xFFFF,
                 rand() & 0xFFFF, rand());
        return std::string(uuid);
    }

    static bool uploadFile(const std::string& filename, const char* data, size_t size) {
        // Check file size
        if (size > MAX_FILE_SIZE) {
            std::cerr << "Error: File too large" << std::endl;
            return false;
        }

        // Validate file type
        if (!isValidFile(filename)) {
            std::cerr << "Error: Invalid file type" << std::endl;
            return false;
        }

        // Create upload directory
        mkdir(UPLOAD_DIR.c_str(), 0755);

        // Generate safe filename
        std::string safeName = sanitizeFilename(filename);
        std::string uniqueName = generateUUID() + "_" + safeName;
        std::string filepath = UPLOAD_DIR + uniqueName;

        // Write file
        std::ofstream outFile(filepath, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error: Could not create file" << std::endl;
            return false;
        }

        outFile.write(data, size);
        outFile.close();

        std::cout << "File uploaded successfully: " << uniqueName << std::endl;
        return true;
    }
};

const std::string Lse71::UPLOAD_DIR = "./uploads/";
std::set<std::string> Lse71::ALLOWED_EXTENSIONS = {"txt", "pdf", "jpg", "jpeg", "png", "gif", "doc", "docx"};

int main() {
    srand(time(NULL));

    std::cout << "=== Secure File Upload System ===" << std::endl;
    std::cout << "\\nRunning test cases...\\n" << std::endl;

    // Test case 1: Valid text file
    std::cout << "Test 1: Valid .txt file" << std::endl;
    const char* testData1 = "Hello, this is test data!";
    Lse71::uploadFile("test1.txt", testData1, strlen(testData1));

    // Test case 2: Valid image file
    std::cout << "\\nTest 2: Valid .jpg file" << std::endl;
    const char* testData2 = "Fake image data";
    Lse71::uploadFile("photo.jpg", testData2, strlen(testData2));

    // Test case 3: Invalid file type
    std::cout << "\\nTest 3: Invalid .exe file" << std::endl;
    const char* testData3 = "Malicious data";
    Lse71::uploadFile("virus.exe", testData3, strlen(testData3));

    // Test case 4: File with special characters
    std::cout << "\\nTest 4: File with special characters" << std::endl;
    const char* testData4 = "Test data";
    Lse71::uploadFile("test@#$%.txt", testData4, strlen(testData4));

    // Test case 5: Valid PDF file
    std::cout << "\\nTest 5: Valid .pdf file" << std::endl;
    const char* testData5 = "PDF content";
    Lse71::uploadFile("document.pdf", testData5, strlen(testData5));

    return 0;
}
