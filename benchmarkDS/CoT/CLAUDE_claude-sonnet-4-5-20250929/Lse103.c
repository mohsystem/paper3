
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int validatePath(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        fprintf(stderr, "Invalid filename\\n");
        return 0;
    }
    
    // Security: Check for path traversal
    if (strstr(filename, "..") != NULL) {
        fprintf(stderr, "Path traversal attempt detected\\n");
        return 0;
    }
    
    return 1;
}

int createFileWithFlags(const char* filename) {
    if (!validatePath(filename)) {
        return 0;
    }
    
    // O_CREAT | O_EXCL | O_WRONLY - fails if file exists
    // 0600 permissions: owner read/write only
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0600);
    
    if (fd == -1) {
        fprintf(stderr, "Error: %s\\n", strerror(errno));
        return 0;
    }
    
    close(fd);
    printf("File created successfully: %s\\n", filename);
    return 1;
}

int truncateAndWriteFile(const char* filename) {
    if (!validatePath(filename)) {
        return 0;
    }
    
    // O_CREAT | O_TRUNC | O_WRONLY - truncates if exists
    int fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0600);
    
    if (fd == -1) {
        fprintf(stderr, "Error: %s\\n", strerror(errno));
        return 0;
    }
    
    close(fd);
    printf("File truncated successfully: %s\\n", filename);
    return 1;
}

int main() {
    printf("=== Test Case 1: Create new file with O_EXCL ===\\n");
    createFileWithFlags("test1.txt");
    
    printf("\\n=== Test Case 2: Try to create existing file (should fail) ===\\n");
    createFileWithFlags("test1.txt");
    
    printf("\\n=== Test Case 3: Truncate existing file ===\\n");
    truncateAndWriteFile("test1.txt");
    
    printf("\\n=== Test Case 4: Create file with invalid path ===\\n");
    createFileWithFlags("../../../etc/passwd");
    
    printf("\\n=== Test Case 5: Create file with valid name ===\\n");
    createFileWithFlags("test2.txt");
    
    // Cleanup
    unlink("test1.txt");
    unlink("test2.txt");
    
    return 0;
}
