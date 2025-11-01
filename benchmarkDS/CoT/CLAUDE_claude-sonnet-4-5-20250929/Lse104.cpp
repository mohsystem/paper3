
#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void writeConfigToFile(const std::string& filename, const std::string& content) {
    if (filename.empty()) {
        std::cerr << "Error: Filename cannot be empty" << std::endl;
        return;
    }
    
    // Open file with O_RDWR (read/write), O_CREAT (create if not exists)
    // Set permissions to 0600 (owner read/write only)
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    
    if (fd == -1) {
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return;
    }
    
    // Write content to file
    ssize_t bytes_written = write(fd, content.c_str(), content.length());
    
    if (bytes_written == -1) {
        std::cerr << "Error writing to file: " << strerror(errno) << std::endl;
    } else if (static_cast<size_t>(bytes_written) != content.length()) {
        std::cerr << "Warning: Not all bytes were written" << std::endl;
    } else {
        std::cout << "Successfully wrote to file: " << filename << std::endl;
    }
    
    // Always close the file descriptor
    if (close(fd) == -1) {
        std::cerr << "Error closing file: " << strerror(errno) << std::endl;
    }
}

int main() {
    std::cout << "Test Case 1: Normal write" << std::endl;
    writeConfigToFile("config1.txt", "important_config");
    
    std::cout << "\\nTest Case 2: Empty content" << std::endl;
    writeConfigToFile("config2.txt", "");
    
    std::cout << "\\nTest Case 3: Long content" << std::endl;
    writeConfigToFile("config3.txt", "important_config_with_more_data_12345");
    
    std::cout << "\\nTest Case 4: Special characters" << std::endl;
    writeConfigToFile("config4.txt", "important_config\\nline2\\ttab");
    
    std::cout << "\\nTest Case 5: Overwrite existing" << std::endl;
    writeConfigToFile("config1.txt", "important_config_updated");
    
    return 0;
}
