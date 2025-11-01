
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

bool writeImportantConfig(const char* content) {
    const char* filename = "important_config";
    
    // Open file with O_CREAT flag
    // Permissions: owner read/write, group read, others read (0644)
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (fd == -1) {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return false;
    }
    
    // Write content to file
    size_t len = strlen(content);
    ssize_t written = write(fd, content, len);
    
    // Close file descriptor
    int closeResult = close(fd);
    
    if (written == -1 || written != static_cast<ssize_t>(len)) {
        std::cerr << "Error writing to file" << std::endl;
        return false;
    }
    
    if (closeResult == -1) {
        std::cerr << "Error closing file" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    // Test cases
    std::cout << "Test 1: " << writeImportantConfig("important_config") << std::endl;
    std::cout << "Test 2: " << writeImportantConfig("test_data_1") << std::endl;
    std::cout << "Test 3: " << writeImportantConfig("configuration_value") << std::endl;
    std::cout << "Test 4: " << writeImportantConfig("secure_settings") << std::endl;
    std::cout << "Test 5: " << writeImportantConfig("") << std::endl;
    
    return 0;
}
