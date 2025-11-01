
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#define chmod _chmod
#endif

void createSecureFile(const std::string& filename, const std::string& content) {
    try {
        // Create file and write content
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create file " << filename << std::endl;
            return;
        }
        file << content;
        file.close();
        
        // Set restrictive permissions (owner read/write only - 0600)
#ifdef _WIN32
        // Windows: use _chmod with limited options
        chmod(filename.c_str(), _S_IREAD | _S_IWRITE);
        std::cout << "File permissions set to owner-only (Windows): " << filename << std::endl;
#else
        // Unix/Linux: use chmod with full POSIX permissions
        chmod(filename.c_str(), S_IRUSR | S_IWUSR);
        
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
                      << " (" << filename << ")" << std::endl;
        }
#endif
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Create secret.txt with encrypted data
    createSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Create another secure file
    createSecureFile("test1.txt", "SecureData123");
    
    // Test case 3: Create file with different content
    createSecureFile("test2.txt", "ConfidentialInfo");
    
    // Test case 4: Create file with empty content
    createSecureFile("test3.txt", "");
    
    // Test case 5: Create file with multi-line content
    createSecureFile("test4.txt", "Line1\\nLine2\\nLine3");
    
    return 0;
}
