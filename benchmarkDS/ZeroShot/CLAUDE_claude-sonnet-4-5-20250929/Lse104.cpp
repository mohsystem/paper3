
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>

void writeToFile(const char* filename, const char* content) {
    // Open file with O_RDWR (read/write) and O_CREAT (create if not exists)
    // 0644 sets file permissions
    int fd = open(filename, O_RDWR | O_CREAT, 0644);
    
    if (fd == -1) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    // Write the content to the file
    ssize_t bytes_written = write(fd, content, strlen(content));
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file: " << filename << std::endl;
    } else {
        std::cout << "Successfully wrote to file: " << filename << std::endl;
    }
    
    // Close the file descriptor
    close(fd);
}

int main() {
    // Test case 1
    writeToFile("config1.txt", "important_config");
    
    // Test case 2
    writeToFile("config2.txt", "database_settings=localhost:5432");
    
    // Test case 3
    writeToFile("config3.txt", "api_key=secret123");
    
    // Test case 4
    writeToFile("config4.txt", "timeout=30");
    
    // Test case 5
    writeToFile("config5.txt", "debug_mode=true");
    
    return 0;
}
