
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>

void writeImportantConfig(const char* filename, const char* content) {
    // Open file with O_CREAT flag, creating if it doesn't exist
    // Permissions: owner read/write, group read, others read
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (fd == -1) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    // Write content to file
    ssize_t bytes_written = write(fd, content, strlen(content));
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file" << std::endl;
    }
    
    // Close file descriptor
    close(fd);
}

int main() {
    // Test case 1
    writeImportantConfig("important_config", "important_config");
    
    // Test case 2
    writeImportantConfig("test_config1.txt", "Configuration data 1");
    
    // Test case 3
    writeImportantConfig("test_config2.txt", "Configuration data 2");
    
    // Test case 4
    writeImportantConfig("test_config3.txt", "Secure configuration settings");
    
    // Test case 5
    writeImportantConfig("test_config4.txt", "Application configuration");
    
    return 0;
}
