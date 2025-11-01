
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

bool createImportantConfig() {
    #ifdef _WIN32
    // Windows implementation
    std::ofstream file("important_config");
    if (file.is_open()) {
        file.close();
        return true;
    }
    return false;
    #else
    // Unix-like systems implementation
    int fd = open("important_config", O_CREAT | O_WRONLY | O_EXCL, 0600);
    if (fd != -1) {
        close(fd);
        return true;
    }
    return false;
    #endif
}

int main() {
    // Test case 1
    std::cout << "Test 1: Creating important_config file" << std::endl;
    std::cout << "Result: " << (createImportantConfig() ? "true" : "false") << std::endl;
    
    // Test case 2
    std::cout << "\\nTest 2: Checking if file exists" << std::endl;
    std::ifstream checkFile("important_config");
    std::cout << "File exists: " << (checkFile.good() ? "true" : "false") << std::endl;
    checkFile.close();
    
    // Test case 3
    std::cout << "\\nTest 3: Checking file permissions" << std::endl;
    #ifndef _WIN32
    struct stat fileStat;
    if (stat("important_config", &fileStat) == 0) {
        std::cout << "File permissions (octal): " << (fileStat.st_mode & 0777) << std::endl;
    }
    #endif
    
    // Test case 4
    std::cout << "\\nTest 4: Attempting to create file again" << std::endl;
    std::cout << "Result: " << (createImportantConfig() ? "true" : "false") << std::endl;
    
    // Test case 5 - Cleanup
    std::cout << "\\nTest 5: Cleanup - deleting file" << std::endl;
    std::cout << "Deleted: " << (remove("important_config") == 0 ? "true" : "false") << std::endl;
    
    return 0;
}
