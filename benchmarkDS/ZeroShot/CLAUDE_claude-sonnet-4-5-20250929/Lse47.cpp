
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>

class Lse47 {
private:
    static const std::string IMAGES_FOLDER;
    
    static bool fileExists(const std::string& path) {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
    }
    
    static bool isValidFilename(const std::string& filename) {
        // Prevent directory traversal attacks
        if (filename.find("..") != std::string::npos ||
            filename.find("/") != std::string::npos ||
            filename.find("\\\\") != std::string::npos) {\n            return false;\n        }\n        return true;\n    }\n    \npublic:\n    static bool serveImage(const std::string& filename, std::ostream& output) {\n        // Validate filename\n        if (!isValidFilename(filename)) {\n            std::cerr << "Error: Invalid filename" << std::endl;\n            return false;\n        }\n        \n        // Build file path\n        std::string filePath = IMAGES_FOLDER + "/" + filename;\n        \n        // Check if file exists\n        if (!fileExists(filePath)) {\n            std::cerr << "Error: File not found - " << filePath << std::endl;\n            return false;\n        }\n        \n        // Open and read file\n        std::ifstream file(filePath, std::ios::binary);\n        if (!file.is_open()) {\n            std::cerr << "Error: Cannot open file" << std::endl;\n            return false;\n        }\n        \n        // Send file to output stream\n        output << file.rdbuf();\n        file.close();\n        \n        std::cout << "File served successfully: " << filename << std::endl;\n        return true;\n    }\n};\n\nconst std::string Lse47::IMAGES_FOLDER = "images";\n\nint main() {\n    std::cout << "Image Server Test Cases:" << std::endl;\n    std::cout << "=========================" << std::endl;\n    \n    // Test case 1\n    std::cout << "\
Test 1: Serve test.jpg" << std::endl;\n    Lse47::serveImage("test.jpg", std::cout);\n    \n    // Test case 2\n    std::cout << "\
Test 2: Serve logo.png" << std::endl;\n    Lse47::serveImage("logo.png", std::cout);\n    \n    // Test case 3\n    std::cout << "\
Test 3: Serve banner.gif" << std::endl;\n    Lse47::serveImage("banner.gif", std::cout);\n    \n    // Test case 4 - Invalid path (directory traversal attempt)\n    std::cout << "\
Test 4: Attempt directory traversal" << std::endl;\n    Lse47::serveImage("../secret.txt", std::cout);\n    \n    // Test case 5 - Non-existent file\n    std::cout << "\
Test 5: Request non-existent file" << std::endl;\n    Lse47::serveImage("nonexistent.jpg", std::cout);
    
    return 0;
}
