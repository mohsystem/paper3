
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

void createAndSecureFile(const std::string& filename, const std::string& content) {
    try {
        // Create file and write content
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return;
        }
        file << content;
        file.close();
        
        // Change permissions to restrictive
#ifdef _WIN32
        // Windows implementation
        if (_chmod(filename.c_str(), _S_IREAD | _S_IWRITE) != 0) {
            std::cerr << "Error: Could not change permissions" << std::endl;
            return;
        }
        std::cout << "Permissions for " << filename << ": Owner read/write only" << std::endl;
#else
        // Unix/Linux implementation (0600 - owner read/write only)
        if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) != 0) {
            std::cerr << "Error: Could not change permissions" << std::endl;
            return;
        }
        
        // Print permissions
        struct stat fileStat;
        if (stat(filename.c_str(), &fileStat) == 0) {
            std::cout << "Permissions for " << filename << ": " 
                      << std::oct << (fileStat.st_mode & 0777) << std::dec << std::endl;
        }
#endif
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Create secret.txt with encrypted content
    std::cout << "Test Case 1:" << std::endl;
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    std::cout << std::endl;
    
    // Test case 2: Create another secure file
    std::cout << "Test Case 2:" << std::endl;
    createAndSecureFile("test1.txt", "Confidential Data");
    std::cout << std::endl;
    
    // Test case 3: Create file with different content
    std::cout << "Test Case 3:" << std::endl;
    createAndSecureFile("test2.txt", "Sensitive Information");
    std::cout << std::endl;
    
    // Test case 4: Create file with empty content
    std::cout << "Test Case 4:" << std::endl;
    createAndSecureFile("test3.txt", "");
    std::cout << std::endl;
    
    // Test case 5: Create file with special characters
    std::cout << "Test Case 5:" << std::endl;
    createAndSecureFile("test4.txt", "Special@#$%^&*()");
    std::cout << std::endl;
    
    return 0;
}
