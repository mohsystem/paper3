
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

bool createSecureConfigFile(const char* filename) {
    #ifdef _WIN32
        // Windows implementation
        FILE* file = fopen(filename, "w");
        if (file != NULL) {
            fclose(file);
            return true;
        }
        return false;
    #else
        // Unix/Linux implementation with secure permissions
        // Create file with permissions 0600 (owner read/write only)
        int fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, S_IRUSR | S_IWUSR);
        
        if (fd == -1) {
            return false;
        }
        
        close(fd);
        
        // Explicitly set permissions to ensure security
        chmod(filename, S_IRUSR | S_IWUSR);
        
        return true;
    #endif
}

int main() {
    // Test case 1: Create important_config file
    printf("Test 1 - Create important_config: %s\\n", 
           createSecureConfigFile("important_config") ? "true" : "false");
    
    // Test case 2: Try to create the same file again
    printf("Test 2 - Create same file again: %s\\n", 
           createSecureConfigFile("important_config") ? "true" : "false");
    
    // Test case 3: Create config file with different name
    printf("Test 3 - Create test_config: %s\\n", 
           createSecureConfigFile("test_config") ? "true" : "false");
    
    // Test case 4: Create config in current directory
    printf("Test 4 - Create app_config: %s\\n", 
           createSecureConfigFile("app_config") ? "true" : "false");
    
    // Test case 5: Create config with different name
    printf("Test 5 - Create secure_settings: %s\\n", 
           createSecureConfigFile("secure_settings") ? "true" : "false");
    
    return 0;
}
