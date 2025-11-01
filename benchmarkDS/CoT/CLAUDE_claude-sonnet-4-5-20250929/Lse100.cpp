
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

bool createSecureConfigFile(const std::string& filename) {
    #ifdef _WIN32
    // Windows implementation
    std::ofstream file(filename);
    if (file.is_open()) {
        file.close();
        return true;
    }
    return false;
    #else
    // Unix/Linux implementation with secure permissions
    int fd = open(filename.c_str(), O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        close(fd);
        return true;
    }
    std::cerr << "Error creating file: " << filename << std::endl;
    return false;
    #endif
}

int main() {
    // Test case 1: Create file with default name
    std::cout << "Test 1: " << createSecureConfigFile("important_config") << std::endl;
    
    // Test case 2: Create file with different name
    std::cout << "Test 2: " << createSecureConfigFile("important_config_2") << std::endl;
    
    // Test case 3: Try to create existing file
    std::cout << "Test 3: " << createSecureConfigFile("important_config") << std::endl;
    
    // Test case 4: Create file in current directory
    std::cout << "Test 4: " << createSecureConfigFile("./config_test") << std::endl;
    
    // Test case 5: Create file with specific path
    std::cout << "Test 5: " << createSecureConfigFile("important_config_final") << std::endl;
    
    return 0;
}
