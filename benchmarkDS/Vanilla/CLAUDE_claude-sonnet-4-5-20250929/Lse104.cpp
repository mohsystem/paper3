
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

void writeToFile(const char* filename, const char* content) {
    // Open file with O_RDWR | O_CREAT flags
    // 0666 is the file permission
    int fd = open(filename, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        std::cerr << "Error opening file" << std::endl;
        return;
    }
    write(fd, content, strlen(content));
    close(fd);
}

int main() {
    // Test case 1
    writeToFile("test1.txt", "important_config");
    
    // Test case 2
    writeToFile("test2.txt", "another important_config");
    
    // Test case 3
    writeToFile("test3.txt", "config data 123");
    
    // Test case 4
    writeToFile("test4.txt", "sensitive information");
    
    // Test case 5
    writeToFile("test5.txt", "final test config");
    
    std::cout << "All test cases completed." << std::endl;
    return 0;
}
