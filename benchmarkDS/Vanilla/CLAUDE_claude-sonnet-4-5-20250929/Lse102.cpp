
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>

void writeImportantConfig(const char* filename, const char* content) {
    // O_CREAT: create if doesn't exist, O_RDWR: read and write, O_TRUNC: truncate if exists
    // S_IRUSR | S_IWUSR: owner read/write, S_IRGRP: group read, S_IROTH: others read
    int fd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (fd != -1) {
        write(fd, content, strlen(content));
        close(fd);
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1
    writeImportantConfig("important_config", "important_config");
    
    // Test case 2
    writeImportantConfig("test_config1.txt", "Configuration data 1");
    
    // Test case 3
    writeImportantConfig("test_config2.txt", "Configuration data 2");
    
    // Test case 4
    writeImportantConfig("test_config3.txt", "Configuration data 3");
    
    // Test case 5
    writeImportantConfig("test_config4.txt", "Configuration data 4");
    
    std::cout << "Files created successfully" << std::endl;
    
    return 0;
}
