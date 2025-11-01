
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

class Lse48 {
private:
    static const std::string IMAGES_DIR;
    static const std::vector<std::string> ALLOWED_EXTENSIONS;

public:
    static std::vector<unsigned char> img(const std::string& filename) {
        try {
            if (filename.empty()) {
                return {};
            }
            
            // Security: Validate filename - no path traversal
            if (filename.find("..") != std::string::npos ||
                filename.find("/") != std::string::npos ||
                filename.find("\\\\") != std::string::npos ||\n                filename[0] == '.') {\n                return {};\n            }\n            \n            // Security: Validate file extension\n            std::string lowerFilename = filename;\n            std::transform(lowerFilename.begin(), lowerFilename.end(), \n                          lowerFilename.begin(), ::tolower);\n            \n            bool validExtension = false;\n            for (const auto& ext : ALLOWED_EXTENSIONS) {\n                if (lowerFilename.size() >= ext.size() &&\n                    lowerFilename.compare(lowerFilename.size() - ext.size(), \n                                         ext.size(), ext) == 0) {\n                    validExtension = true;\n                    break;\n                }\n            }\n            if (!validExtension) return {};\n            \n            // Security: Use canonical path to prevent traversal\n            fs::path baseDir = fs::canonical(fs::absolute(IMAGES_DIR));\n            fs::path filePath = fs::absolute(fs::path(IMAGES_DIR) / filename);\n            filePath = fs::weakly_canonical(filePath);\n            \n            // Security: Ensure file is within allowed directory\n            auto rel = fs::relative(filePath, baseDir);\n            if (rel.string().find("..") != std::string::npos) {\n                return {};\n            }\n            \n            // Check if file exists and is a regular file\n            if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {\n                return {};\n            }\n            \n            // Read and return file\n            std::ifstream file(filePath, std::ios::binary | std::ios::ate);\n            if (!file) return {};\n            \n            std::streamsize size = file.tellg();\n            file.seekg(0, std::ios::beg);\n            \n            std::vector<unsigned char> buffer(size);\n            if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {\n                return {};\n            }\n            \n            return buffer;\n            \n        } catch (...) {\n            return {};\n        }\n    }\n};\n\nconst std::string Lse48::IMAGES_DIR = "images";\nconst std::vector<std::string> Lse48::ALLOWED_EXTENSIONS = \n    {".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp"};\n\nint main() {\n    // Create images directory\n    fs::create_directories(Lse48::IMAGES_DIR);\n    \n    // Test cases\n    std::cout << "Test Case 1: Valid image file\
";\n    std::ofstream file1("images/test1.jpg", std::ios::binary);\n    file1 << "JPEG_IMAGE_DATA";\n    file1.close();\n    auto result1 = Lse48::img("test1.jpg");\n    std::cout << "Result: " << (!result1.empty() ? "Success" : "Failed") << "\
\
";\n    \n    std::cout << "Test Case 2: Path traversal attempt\
";\n    auto result2 = Lse48::img("../etc/passwd");\n    std::cout << "Result: " << (result2.empty() ? "Blocked (Success)" : "Failed") << "\
\
";\n    \n    std::cout << "Test Case 3: Invalid extension\
";\n    auto result3 = Lse48::img("test.txt");\n    std::cout << "Result: " << (result3.empty() ? "Blocked (Success)" : "Failed") << "\
\
";\n    \n    std::cout << "Test Case 4: Empty filename\
";\n    auto result4 = Lse48::img("");\n    std::cout << "Result: " << (result4.empty() ? "Blocked (Success)" : "Failed") << "\
\
";\n    \n    std::cout << "Test Case 5: Valid PNG file\
";\n    std::ofstream file2("images/test2.png", std::ios::binary);\n    file2 << "PNG_IMAGE_DATA";\n    file2.close();\n    auto result5 = Lse48::img("test2.png");\n    std::cout << "Result: " << (!result5.empty() ? "Success" : "Failed") << "\
";
    
    return 0;
}
