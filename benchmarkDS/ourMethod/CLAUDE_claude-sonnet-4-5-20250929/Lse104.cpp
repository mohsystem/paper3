
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <limits.h>

// Secure file write function with proper validation and error handling
bool writeConfigFile(const char* filename, const char* content) {
    if (filename == nullptr || content == nullptr) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return false;
    }
    
    // Validate filename to prevent directory traversal
    if (strstr(filename, "..") != nullptr || filename[0] == '/') {
        fprintf(stderr, "Error: Invalid filename\\n");
        return false;
    }
    
    // Get current working directory as base
    char base_path[PATH_MAX];
    if (getcwd(base_path, sizeof(base_path)) == nullptr) {
        fprintf(stderr, "Error: Cannot get current directory\\n");
        return false;
    }
    
    // Open base directory for openat
    int dirfd = open(base_path, O_RDONLY | O_CLOEXEC | O_DIRECTORY);
    if (dirfd < 0) {
        fprintf(stderr, "Error: Cannot open base directory\\n");
        return false;
    }
    
    // Create temp file with restrictive permissions (0600) and O_NOFOLLOW to prevent symlink attacks
    int fd = openat(dirfd, "temp_config.tmp", 
                    O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 
                    0600);
    
    if (fd < 0) {
        close(dirfd);
        fprintf(stderr, "Error: Cannot create temp file\\n");
        return false;
    }
    
    // Verify it's a regular file using fstat on the opened descriptor
    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        unlinkat(dirfd, "temp_config.tmp", 0);
        close(dirfd);
        fprintf(stderr, "Error: Not a regular file\\n");
        return false;
    }
    
    // Write content
    size_t content_len = strlen(content);
    ssize_t written = write(fd, content, content_len);
    
    if (written < 0 || (size_t)written != content_len) {
        close(fd);
        unlinkat(dirfd, "temp_config.tmp", 0);
        close(dirfd);
        fprintf(stderr, "Error: Write failed\\n");
        return false;
    }
    
    // Ensure data is written to disk
    if (fsync(fd) != 0) {
        close(fd);
        unlinkat(dirfd, "temp_config.tmp", 0);
        close(dirfd);
        fprintf(stderr, "Error: Sync failed\\n");
        return false;
    }
    
    close(fd);
    
    // Atomic rename to final location
    if (renameat(dirfd, "temp_config.tmp", dirfd, filename) != 0) {
        unlinkat(dirfd, "temp_config.tmp", 0);
        close(dirfd);
        fprintf(stderr, "Error: Rename failed\\n");
        return false;
    }
    
    close(dirfd);
    return true;
}

int main() {
    // Test case 1: Normal write
    printf("Test 1: %s\\n", writeConfigFile("config1.txt", "important_config") ? "true" : "false");
    
    // Test case 2: Empty content
    printf("Test 2: %s\\n", writeConfigFile("config2.txt", "") ? "true" : "false");
    
    // Test case 3: Multi-line content
    printf("Test 3: %s\\n", writeConfigFile("config3.txt", "line1\\nline2\\nline3") ? "true" : "false");
    
    // Test case 4: nullptr filename (should fail)
    printf("Test 4: %s\\n", writeConfigFile(nullptr, "important_config") ? "true" : "false");
    
    // Test case 5: Path traversal attempt (should fail)
    printf("Test 5: %s\\n", writeConfigFile("../../../etc/passwd", "important_config") ? "true" : "false");
    
    return 0;
}
