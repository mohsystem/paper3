
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int createSecureConfigFile(const char* filename) {
    #ifdef _WIN32
    FILE* file = fopen(filename, "w");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
    #else
    // Unix/Linux implementation with secure permissions (0600)
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd != -1) {
        close(fd);
        return 1;
    }
    fprintf(stderr, "Error creating file: %s\\n", filename);
    return 0;
    #endif
}

int main() {
    // Test case 1: Create file with default name
    printf("Test 1: %d\\n", createSecureConfigFile("important_config"));
    
    // Test case 2: Create file with different name
    printf("Test 2: %d\\n", createSecureConfigFile("important_config_2"));
    
    // Test case 3: Try to create existing file
    printf("Test 3: %d\\n", createSecureConfigFile("important_config"));
    
    // Test case 4: Create file in current directory
    printf("Test 4: %d\\n", createSecureConfigFile("./config_test"));
    
    // Test case 5: Create file with specific path
    printf("Test 5: %d\\n", createSecureConfigFile("important_config_final"));
    
    return 0;
}
