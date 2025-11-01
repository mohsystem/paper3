
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void writeToConfigFile() {
    const char* filename = "important_config";
    const char* content = "important_config";
    
    #ifdef _WIN32
        // Windows implementation
        FILE* file = fopen(filename, "w");
        if (file != NULL) {
            fprintf(file, "%s", content);
            fclose(file);
            printf("File written successfully\\n");
        } else {
            fprintf(stderr, "Error opening file\\n");
        }
    #else
        // Unix/Linux implementation with secure permissions (0600)
        int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        
        if (fd == -1) {
            fprintf(stderr, "Error opening file: %s\\n", strerror(errno));
            return;
        }
        
        ssize_t bytes_written = write(fd, content, strlen(content));
        
        if (bytes_written == -1) {
            fprintf(stderr, "Error writing to file: %s\\n", strerror(errno));
        } else {
            printf("File written successfully with secure permissions (0600)\\n");
        }
        
        close(fd);
    #endif
}

int main() {
    printf("Test Case 1: Writing to config file\\n");
    writeToConfigFile();
    
    printf("\\nTest Case 2: Verify file exists\\n");
    FILE* file = fopen("important_config", "r");
    printf("File exists: %s\\n", file != NULL ? "true" : "false");
    if (file != NULL) fclose(file);
    
    printf("\\nTest Case 3: Verify file content\\n");
    file = fopen("important_config", "r");
    if (file != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), file) != NULL) {
            printf("File content: %s\\n", buffer);
        }
        fclose(file);
    }
    
    printf("\\nTest Case 4: Multiple writes\\n");
    writeToConfigFile();
    
    printf("\\nTest Case 5: Check file permissions\\n");
    #ifndef _WIN32
    struct stat st;
    if (stat("important_config", &st) == 0) {
        printf("File permissions: %o\\n", st.st_mode & 0777);
    }
    #else
    printf("Permission check not available on Windows\\n");
    #endif
    
    return 0;
}
