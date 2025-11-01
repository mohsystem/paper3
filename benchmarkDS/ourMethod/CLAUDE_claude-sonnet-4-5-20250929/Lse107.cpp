
#include <iostream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

void createAndSecureFile(const char* filename, const char* content) {
    if (!filename || !content) {
        std::cerr << "Invalid parameters" << std::endl;
        return;
    }
    
    // Create file with restrictive permissions (0600 - owner read/write only)
    // O_CREAT|O_EXCL ensures atomic creation and fails if file exists
    // O_WRONLY for write-only, O_CLOEXEC prevents descriptor leaks to child processes
    // O_NOFOLLOW prevents following symlinks for security
    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC;
    #ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
    #endif
    
    mode_t mode = S_IRUSR | S_IWUSR;  // 0600 - owner read/write only
    
    // Open file with secure flags and restrictive permissions from creation
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        std::cerr << "Error creating file: " << strerror(errno) << std::endl;
        return;
    }
    
    // Write content to file
    size_t content_len = strlen(content);
    ssize_t written = write(fd, content, content_len);
    
    if (written < 0 || static_cast<size_t>(written) != content_len) {
        std::cerr << "Error writing to file" << std::endl;
        close(fd);
        return;
    }
    
    // Ensure data is flushed to disk
    if (fsync(fd) < 0) {
        std::cerr << "Error syncing file" << std::endl;
    }
    
    // Use fstat on the open descriptor to avoid TOCTOU
    struct stat file_stat;
    if (fstat(fd, &file_stat) < 0) {
        std::cerr << "Error getting file stats" << std::endl;
        close(fd);
        return;
    }
    
    // Close the file descriptor
    close(fd);
    
    // Print permissions in octal format
    mode_t perms = file_stat.st_mode & 0777;
    std::cout << "File permissions (octal): 0" << std::oct << perms << std::dec << std::endl;
    
    // Print permissions in symbolic format
    std::cout << "File permissions (symbolic): ";
    std::cout << ((file_stat.st_mode & S_IRUSR) ? "r" : "-");
    std::cout << ((file_stat.st_mode & S_IWUSR) ? "w" : "-");
    std::cout << ((file_stat.st_mode & S_IXUSR) ? "x" : "-");
    std::cout << ((file_stat.st_mode & S_IRGRP) ? "r" : "-");
    std::cout << ((file_stat.st_mode & S_IWGRP) ? "w" : "-");
    std::cout << ((file_stat.st_mode & S_IXGRP) ? "x" : "-");
    std::cout << ((file_stat.st_mode & S_IROTH) ? "r" : "-");
    std::cout << ((file_stat.st_mode & S_IWOTH) ? "w" : "-");
    std::cout << ((file_stat.st_mode & S_IXOTH) ? "x" : "-");
    std::cout << std::endl;
}

int main() {
    // Test case 1: Standard operation
    std::cout << "Test 1: Creating secret.txt" << std::endl;
    unlink("secret.txt");  // Remove if exists
    createAndSecureFile("secret.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 2: Different filename
    std::cout << "\\nTest 2: Creating secret2.txt" << std::endl;
    unlink("secret2.txt");
    createAndSecureFile("secret2.txt", "U2FsdGVkX1/R+WzJcxgvX/Iw==");
    
    // Test case 3: Different content
    std::cout << "\\nTest 3: Creating secret3.txt with different content" << std::endl;
    unlink("secret3.txt");
    createAndSecureFile("secret3.txt", "TestContent123");
    
    // Test case 4: Empty content
    std::cout << "\\nTest 4: Creating secret4.txt with empty content" << std::endl;
    unlink("secret4.txt");
    createAndSecureFile("secret4.txt", "");
    
    // Test case 5: Special characters
    std::cout << "\\nTest 5: Creating secret5.txt with special characters" << std::endl;
    unlink("secret5.txt");
    createAndSecureFile("secret5.txt", "Special!@#$%^&*()");
    
    return 0;
}
