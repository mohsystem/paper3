
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

void createAndSecureFile(const std::string& filename, const std::string& content) {
    try {
        // Create file and write content
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Error: Could not create file" << std::endl;
            return;
        }
        file << content;
        file.close();
        
        // Change permissions to restrictive (owner read/write only: 0600)
        #ifdef _WIN32
            std::cout << "Windows does not support POSIX permissions" << std::endl;
        #else
            if (chmod(filename.c_str(), S_IRUSR | S_IWUSR) != 0) {
                std::cout << "Error: Could not change permissions" << std::endl;
                return;
            }
            
            // Print permissions
            struct stat fileStat;
            if (stat(filename.c_str(), &fileStat) == 0) {
                std::cout << "File permissions: " 
                          << ((fileStat.st_mode & S_IRUSR) ? "r" : "-")
                          << ((fileStat.st_mode & S_IWUSR) ? "w" : "-")
                          << ((fileStat.st_mode & S_IXUSR) ? "x" : "-")
                          << ((fileStat.st_mode & S_IRGRP) ? "r" : "-")
                          << ((fileStat.st_mode & S_IWGRP) ? "w" : "-")
                          << ((fileStat.st_mode & S_IXGRP) ? "x" : "-")
                          << ((fileStat.st_mode & S_IROTH) ? "r" : "-")
                          << ((fileStat.st_mode & S_IWOTH) ? "w" : "-")
                          << ((fileStat.st_mode & S_IXOTH) ? "x" : "-")
                          << std::endl;
            }
        #endif
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "\\nTest Case 2:" << std::endl;
    createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "\\nTest Case 3:" << std::endl;
    createAndSecureFile("secret3.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "\\nTest Case 4:" << std::endl;
    createAndSecureFile("secret4.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    std::cout << "\\nTest Case 5:" << std::endl;
    createAndSecureFile("secret5.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    return 0;
}
