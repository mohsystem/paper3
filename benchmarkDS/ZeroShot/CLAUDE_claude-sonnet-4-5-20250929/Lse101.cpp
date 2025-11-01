
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

void writeToFile(const std::string& filename, const std::string& content) {
    // Open file with restricted permissions (owner read/write only - 0600)
    int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    
    write(fd, content.c_str(), content.length());
    close(fd);
}

int main() {
    // Test case 1: Write to important_config
    writeToFile("important_config", "important_config");
    
    // Test case 2: Write different content
    writeToFile("test_config_1", "test_content_1");
    
    // Test case 3: Write empty string
    writeToFile("test_config_2", "");
    
    // Test case 4: Write multi-line content
    writeToFile("test_config_3", "line1\\nline2\\nline3");
    
    // Test case 5: Write special characters
    writeToFile("test_config_4", "special!@#$%^&*()");
    
    std::cout << "Files written successfully" << std::endl;
    
    return 0;
}
