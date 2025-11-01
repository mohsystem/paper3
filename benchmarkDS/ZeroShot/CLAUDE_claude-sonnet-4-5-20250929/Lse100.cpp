
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool createSecureConfigFile(const std::string& filename) {
    #ifdef _WIN32
        // Windows implementation
        std::ofstream file(filename, std::ios::out);
        if (file.is_open()) {
            file.close();
            return true;
        }
        return false;
    #else
        // Unix/Linux implementation with secure permissions
        // Create file with permissions 0600 (owner read/write only)
        int fd = open(filename.c_str(), O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
        
        if (fd == -1) {
            return false;
        }
        
        close(fd);
        
        // Explicitly set permissions to ensure security
        chmod(filename.c_str(), S_IRUSR | S_IWUSR);
        
        return true;
    #endif
}

int main() {
    // Test case 1: Create important_config file
    std::cout << "Test 1 - Create important_config: " 
              << (createSecureConfigFile("important_config") ? "true" : "false") << std::endl;
    
    // Test case 2: Try to create the same file again
    std::cout << "Test 2 - Create same file again: " 
              << (createSecureConfigFile("important_config") ? "true" : "false") << std::endl;
    
    // Test case 3: Create config file with different name
    std::cout << "Test 3 - Create test_config: " 
              << (createSecureConfigFile("test_config") ? "true" : "false") << std::endl;
    
    // Test case 4: Create config in current directory
    std::cout << "Test 4 - Create app_config: " 
              << (createSecureConfigFile("app_config") ? "true" : "false") << std::endl;
    
    // Test case 5: Create config with different name
    std::cout << "Test 5 - Create secure_settings: " 
              << (createSecureConfigFile("secure_settings") ? "true" : "false") << std::endl;
    
    return 0;
}
