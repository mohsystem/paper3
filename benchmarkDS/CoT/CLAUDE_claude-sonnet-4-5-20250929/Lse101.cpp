
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

void writeToConfigFile() {
    const char* filename = "important_config";
    const char* content = "important_config";
    
    #ifdef _WIN32
        // Windows implementation
        std::ofstream file(filename, std::ios::out | std::ios::trunc);
        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "File written successfully" << std::endl;
        } else {
            std::cerr << "Error opening file" << std::endl;
        }
    #else
        // Unix/Linux implementation with secure permissions
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        
        if (fd == -1) {
            std::cerr << "Error opening file: " << strerror(errno) << std::endl;
            return;
        }
        
        ssize_t bytes_written = write(fd, content, strlen(content));
        
        if (bytes_written == -1) {
            std::cerr << "Error writing to file: " << strerror(errno) << std::endl;
        } else {
            std::cout << "File written successfully with secure permissions (0600)" << std::endl;
        }
        
        close(fd);
    #endif
}

int main() {
    std::cout << "Test Case 1: Writing to config file" << std::endl;
    writeToConfigFile();
    
    std::cout << "\\nTest Case 2: Verify file exists" << std::endl;
    std::ifstream file("important_config");
    std::cout << "File exists: " << (file.good() ? "true" : "false") << std::endl;
    file.close();
    
    std::cout << "\\nTest Case 3: Verify file content" << std::endl;
    std::ifstream readFile("important_config");
    if (readFile.is_open()) {
        std::string content;
        std::getline(readFile, content);
        std::cout << "File content: " << content << std::endl;
        readFile.close();
    }
    
    std::cout << "\\nTest Case 4: Multiple writes" << std::endl;
    writeToConfigFile();
    
    std::cout << "\\nTest Case 5: Check file size" << std::endl;
    std::ifstream sizeFile("important_config", std::ios::ate);
    std::cout << "File size: " << sizeFile.tellg() << " bytes" << std::endl;
    
    return 0;
}
