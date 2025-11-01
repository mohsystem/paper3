
#include <iostream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class Lse114 {
public:
    // SECURE ALTERNATIVE: Use C++ filesystem library
    static void listDirectorySecure(const std::string& directoryPath) {
        try {
            fs::path path(directoryPath);
            
            // Validate path exists and is a directory
            if (!fs::exists(path) || !fs::is_directory(path)) {
                std::cout << "Invalid directory path: " << directoryPath << std::endl;
                return;
            }
            
            // Securely list directory contents
            for (const auto& entry : fs::directory_iterator(path)) {
                std::cout << entry.path().filename().string() << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cout << "Error accessing directory: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    // WARNING: INSECURE - For educational purposes only
    // DO NOT USE IN PRODUCTION
    static void listDirectoryUnsafe(const std::string& directory) {
        std::cout << "WARNING: system() calls are insecure and vulnerable to injection attacks" << std::endl;
        // Intentionally not implementing: system(("ls " + directory).c_str());
    }
};

int main() {
    std::cout << "=== Secure Directory Listing ===" << std::endl << std::endl;
    
    // Test case 1: Current directory
    std::cout << "Test 1 - Current directory:" << std::endl;
    Lse114::listDirectorySecure(".");
    
    // Test case 2: Parent directory
    std::cout << "\\nTest 2 - Parent directory:" << std::endl;
    Lse114::listDirectorySecure("..");
    
    // Test case 3: Temp directory
    std::cout << "\\nTest 3 - Temp directory:" << std::endl;
    Lse114::listDirectorySecure("/tmp");
    
    // Test case 4: Invalid path
    std::cout << "\\nTest 4 - Invalid path:" << std::endl;
    Lse114::listDirectorySecure("/nonexistent/path");
    
    // Test case 5: Root directory (limited listing)
    std::cout << "\\nTest 5 - Root directory:" << std::endl;
    Lse114::listDirectorySecure("/");
    
    return 0;
}
