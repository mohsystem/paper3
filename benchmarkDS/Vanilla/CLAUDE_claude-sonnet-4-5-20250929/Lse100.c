
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int createImportantConfig() {
    #ifdef _WIN32
    // Windows implementation
    FILE* file = fopen("important_config", "w");
    if (file != NULL) {
        fclose(file);
        return 1;
    }
    return 0;
    #else
    // Unix-like systems implementation
    int fd = open("important_config", O_CREAT | O_WRONLY | O_EXCL, 0600);
    if (fd != -1) {
        close(fd);
        return 1;
    }
    return 0;
    #endif
}

int main() {
    // Test case 1
    printf("Test 1: Creating important_config file\\n");
    printf("Result: %s\\n", createImportantConfig() ? "true" : "false");
    
    // Test case 2
    printf("\\nTest 2: Checking if file exists\\n");
    FILE* checkFile = fopen("important_config", "r");
    printf("File exists: %s\\n", (checkFile != NULL) ? "true" : "false");
    if (checkFile != NULL) {
        fclose(checkFile);
    }
    
    // Test case 3
    printf("\\nTest 3: Checking file permissions\\n");
    #ifndef _WIN32
    struct stat fileStat;
    if (stat("important_config", &fileStat) == 0) {
        printf("File permissions (octal): %o\\n", fileStat.st_mode & 0777);
    }
    #endif
    
    // Test case 4
    printf("\\nTest 4: Attempting to create file again\\n");
    printf("Result: %s\\n", createImportantConfig() ? "true" : "false");
    
    // Test case 5 - Cleanup
    printf("\\nTest 5: Cleanup - deleting file\\n");
    printf("Deleted: %s\\n", (remove("important_config") == 0) ? "true" : "false");
    
    return 0;
}
