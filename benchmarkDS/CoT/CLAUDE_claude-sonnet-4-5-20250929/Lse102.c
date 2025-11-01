
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

int writeImportantConfig(const char* content) {
    const char* filename = "important_config";
    
    // Open file with O_CREAT flag
    // Permissions: owner read/write, group read, others read (0644)
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 
                  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    
    if (fd == -1) {
        fprintf(stderr, "Error opening file: %s\\n", strerror(errno));
        return 0;
    }
    
    // Write content to file
    size_t len = strlen(content);
    ssize_t written = write(fd, content, len);
    
    // Close file descriptor
    int closeResult = close(fd);
    
    if (written == -1 || written != (ssize_t)len) {
        fprintf(stderr, "Error writing to file\\n");
        return 0;
    }
    
    if (closeResult == -1) {
        fprintf(stderr, "Error closing file\\n");
        return 0;
    }
    
    return 1;
}

int main() {
    // Test cases
    printf("Test 1: %d\\n", writeImportantConfig("important_config"));
    printf("Test 2: %d\\n", writeImportantConfig("test_data_1"));
    printf("Test 3: %d\\n", writeImportantConfig("configuration_value"));
    printf("Test 4: %d\\n", writeImportantConfig("secure_settings"));
    printf("Test 5: %d\\n", writeImportantConfig(""));
    
    return 0;
}
