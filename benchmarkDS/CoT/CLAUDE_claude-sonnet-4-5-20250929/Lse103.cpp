
#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

bool validatePath(const std::string& filename) {
    if (filename.empty()) {
        std::cerr << "Invalid filename" << std::endl;
        return false;
    }
    
    // Security: Check for path traversal
    if (filename.find("..") != std::string::npos) {
        std::cerr << "Path traversal attempt detected" << std::endl;
        return false;
    }
    
    return true;
}

bool createFileWithFlags(const std::string& filename) {
    if (!validatePath(filename)) {
        return false;
    }
    
    // O_CREAT | O_EXCL | O_WRONLY - fails if file exists
    // 0600 permissions: owner read/write only
    int fd = open(filename.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0600);
    
    if (fd == -1) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return false;
    }
    
    close(fd);
    std::cout << "File created successfully: " << filename << std::endl;
    return true;
}

bool truncateAndWriteFile(const std::string& filename) {
    if (!validatePath(filename)) {
        return false;
    }
    
    // O_CREAT | O_TRUNC | O_WRONLY - truncates if exists
    int fd = open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
    
    if (fd == -1) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return false;
    }
    
    close(fd);
    std::cout << "File truncated successfully: " << filename << std::endl;
    return true;
}

int main() {
    std::cout << "=== Test Case 1: Create new file with O_EXCL ===" << std::endl;
    createFileWithFlags("test1.txt");
    
    std::cout << "\\n=== Test Case 2: Try to create existing file (should fail) ===" << std::endl;
    createFileWithFlags("test1.txt");
    
    std::cout << "\\n=== Test Case 3: Truncate existing file ===" << std::endl;
    truncateAndWriteFile("test1.txt");
    
    std::cout << "\\n=== Test Case 4: Create file with invalid path ===" << std::endl;
    createFileWithFlags("../../../etc/passwd");
    
    std::cout << "\\n=== Test Case 5: Create file with valid name ===" << std::endl;
    createFileWithFlags("test2.txt");
    
    // Cleanup
    unlink("test1.txt");
    unlink("test2.txt");
    
    return 0;
}
