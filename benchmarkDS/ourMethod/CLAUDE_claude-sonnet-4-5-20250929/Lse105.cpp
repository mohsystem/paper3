
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <iostream>
#include <string>

// Create a file with restrictive permissions (0600 - owner read/write only)
// Uses open() with O_CREAT|O_EXCL for atomic creation and immediate permission setting
int createRestrictiveFile(const char* filename, const char* content) {
    // Define restrictive permissions: owner read/write only (0600)
    // This prevents unauthorized access from other users or groups
    mode_t restrictive_perms = S_IRUSR | S_IWUSR;  // 0600
    
    // Open file with O_CREAT|O_EXCL|O_WRONLY|O_TRUNC
    // O_EXCL ensures atomic creation (fails if exists)
    // O_NOFOLLOW prevents following symlinks (security measure)
    // Set permissions at creation time to avoid TOCTOU
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_NOFOLLOW, restrictive_perms);
    
    if (fd < 0) {
        if (errno == EEXIST) {
            // File exists - remove and recreate for idempotent behavior
            if (unlink(filename) != 0) {
                std::cerr << "Failed to remove existing file: " << strerror(errno) << std::endl;
                return -1;
            }
            fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_NOFOLLOW, restrictive_perms);
            if (fd < 0) {
                std::cerr << "Failed to create file after removal: " << strerror(errno) << std::endl;
                return -1;
            }
        } else {
            std::cerr << "Failed to open file: " << strerror(errno) << std::endl;
            return -1;
        }
    }
    
    // Write content to file
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written < 0 || static_cast<size_t>(bytes_written) != content_len) {
        std::cerr << "Failed to write content: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }
    
    // Explicitly set permissions using fchmod on the open file descriptor
    // This ensures permissions are set correctly regardless of umask
    if (fchmod(fd, restrictive_perms) != 0) {
        std::cerr << "Failed to set permissions: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }
    
    // Close file descriptor
    if (close(fd) != 0) {
        std::cerr << "Failed to close file: " << strerror(errno) << std::endl;
        return -1;
    }
    
    return 0;
}

int main() {
    // Test cases
    const char* test_files[] = {
        "secret.txt",
        "test1_secret.txt",
        "test2_secret.txt",
        "test3_secret.txt",
        "test4_secret.txt"
    };
    
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    
    for (size_t i = 0; i < 5; ++i) {
        if (createRestrictiveFile(test_files[i], content) == 0) {
            std::cout << "Successfully created " << test_files[i] 
                     << " with restrictive permissions (0600)" << std::endl;
        } else {
            std::cerr << "Failed to create " << test_files[i] << std::endl;
        }
    }
    
    return 0;
}
