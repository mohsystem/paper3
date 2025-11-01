
#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Security: Create file with restrictive permissions to prevent unauthorized access
bool createSecureConfigFile(const std::string& filename) {
    // Security: Use O_CREAT|O_EXCL to fail if file exists (prevents TOCTOU race)
    // O_NOFOLLOW prevents following symbolic links
    // O_CLOEXEC prevents file descriptor leaks across exec
    int flags = O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC;
    
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    
    // Security: Set mode to 0600 (owner read/write only) at creation time
    // This is more secure than creating with default permissions and changing later
    mode_t mode = S_IRUSR | S_IWUSR;  // 0600 permissions
    
    // Security: Open file with restrictive permissions atomically
    int fd = open(filename.c_str(), flags, mode);
    
    if (fd == -1) {
        // Security: Log error without exposing sensitive details
        std::cerr << "Error creating file: " << std::strerror(errno) << std::endl;
        return false;
    }
    
    // Security: Verify the opened file is a regular file using fstat
    struct stat st;
    if (fstat(fd, &st) != 0) {
        std::cerr << "Error: fstat failed" << std::endl;
        close(fd);
        return false;
    }
    
    // Security: Ensure it's a regular file, not a symlink or directory
    if (!S_ISREG(st.st_mode)) {
        std::cerr << "Error: Not a regular file" << std::endl;
        close(fd);
        return false;
    }
    
    // Security: Close file descriptor immediately after creation
    close(fd);
    return true;
}

int main() {
    // Test case 1: Create important_config file
    std::cout << "Test 1 - Create important_config: " 
              << (createSecureConfigFile("important_config") ? "Success" : "Failed") 
              << std::endl;
    
    // Test case 2: Attempt to create same file again (should fail)
    std::cout << "Test 2 - Create duplicate (should fail): " 
              << (createSecureConfigFile("important_config") ? "Success" : "Failed") 
              << std::endl;
    
    // Test case 3: Create different config file
    std::cout << "Test 3 - Create test_config1: " 
              << (createSecureConfigFile("test_config1") ? "Success" : "Failed") 
              << std::endl;
    
    // Test case 4: Create another config file
    std::cout << "Test 4 - Create test_config2: " 
              << (createSecureConfigFile("test_config2") ? "Success" : "Failed") 
              << std::endl;
    
    // Test case 5: Create secure settings file
    std::cout << "Test 5 - Create secure_settings: " 
              << (createSecureConfigFile("secure_settings") ? "Success" : "Failed") 
              << std::endl;
    
    // Cleanup test files
    unlink("important_config");
    unlink("test_config1");
    unlink("test_config2");
    unlink("secure_settings");
    
    return 0;
}
