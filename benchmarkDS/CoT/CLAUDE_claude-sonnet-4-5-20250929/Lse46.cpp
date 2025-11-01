
// Note: C++ doesn't have built-in web framework decorators like Python Flask\n// This example uses a simple HTTP server library concept\n// For production, use libraries like Boost.Beast, cpp-httplib, or Pistache\n\n#include <iostream>\n#include <fstream>\n#include <string>\n#include <regex>\n#include <filesystem>\n#include <cstring>\n\n#ifdef _WIN32\n    #include <winsock2.h>\n    #pragma comment(lib, "ws2_32.lib")\n#else\n    #include <sys/socket.h>\n    #include <netinet/in.h>\n    #include <unistd.h>\n#endif\n\nnamespace fs = std::filesystem;\n\nclass Lse46 {\nprivate:\n    static const std::string IMAGES_DIR;\n    static const std::regex SAFE_FILENAME_PATTERN;\n    \n    static bool isValidFilename(const std::string& filename) {\n        return std::regex_match(filename, SAFE_FILENAME_PATTERN);\n    }\n    \n    static std::string extractNameParameter(const std::string& query) {\n        size_t pos = query.find("name=");\n        if (pos != std::string::npos) {\n            size_t start = pos + 5;\n            size_t end = query.find('&', start);
            if (end == std::string::npos) {
                return query.substr(start);
            }
            return query.substr(start, end - start);
        }
        return "";
    }
    
    static std::string readFile(const fs::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return "";
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
    
public:
    static std::string img(const std::string& name) {
        if (name.empty() || !isValidFilename(name)) {
            return "ERROR: Invalid filename";
        }
        
        fs::path imagePath = fs::path(IMAGES_DIR) / name;
        fs::path imagesDir = fs::canonical(fs::path(IMAGES_DIR));
        
        try {
            fs::path resolvedPath = fs::canonical(imagePath);
            
            std::string resolvedStr = resolvedPath.string();
            std::string imagesDirStr = imagesDir.string();
            
            if (resolvedStr.find(imagesDirStr) != 0) {
                return "ERROR: Access denied";
            }
            
            if (!fs::exists(resolvedPath)) {
                return "ERROR: File not found";
            }
            
            return readFile(resolvedPath);
        } catch (const fs::filesystem_error& e) {
            return "ERROR: " + std::string(e.what());
        }
    }
    
    static void setupTestEnvironment() {
        fs::create_directories(IMAGES_DIR);
        
        std::ofstream file1(fs::path(IMAGES_DIR) / "test1.jpg");
        file1 << "Fake JPG content";
        file1.close();
        
        std::ofstream file2(fs::path(IMAGES_DIR) / "test2.png");
        file2 << "Fake PNG content";
        file2.close();
    }
    
    static void runTests() {
        setupTestEnvironment();
        
        std::cout << "Test Cases:\\n";
        std::cout << "1. Valid JPG: " << img("test1.jpg") << "\\n";
        std::cout << "2. Valid PNG: " << img("test2.png") << "\\n";
        std::cout << "3. Path traversal attempt: " << img("../etc/passwd") << "\\n";
        std::cout << "4. Invalid extension: " << img("test.txt") << "\\n";
        std::cout << "5. Non-existent file: " << img("nonexistent.jpg") << "\\n";
    }
};

const std::string Lse46::IMAGES_DIR = "images";
const std::regex Lse46::SAFE_FILENAME_PATTERN("^[a-zA-Z0-9_-]+\\\\.(jpg|jpeg|png|gif|bmp)$");

int main() {
    Lse46::runTests();
    return 0;
}
