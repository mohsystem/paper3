#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>

bool createFileAndWrite(const char* filename, const char* content) {
    // O_RDWR: Open for reading and writing.
    // O_CREAT: Create the file if it does not exist.
    // O_TRUNC: Truncate file to length 0 if it exists.
    // 0644: Permissions for the new file (owner read/write, group/other read).
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0644);
    
    if (fd == -1) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return false;
    }
    
    ssize_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written == -1) {
        fprintf(stderr, "Error writing to file '%s': %s\n", filename, strerror(errno));
        close(fd);
        return false;
    }
    
    close(fd);
    return true;
}

int main() {
    const char* content = "important_config";
    
    printf("--- C Test Cases ---\n");
    
    // Test Case 1: Standard file creation
    const char* file1 = "c_test1.cfg";
    printf("\nTest 1: Writing to '%s'\n", file1);
    if (createFileAndWrite(file1, content)) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    
    // Test Case 2: Different content
    const char* file2 = "c_test2.dat";
    printf("\nTest 2: Writing to '%s'\n", file2);
    if (createFileAndWrite(file2, "important_config for test 2")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    
    // Test Case 3: Path with non-existent directory (should fail)
    const char* file3 = "data/c_test3.txt";
    printf("\nTest 3: Writing to '%s'\n", file3);
    if (createFileAndWrite(file3, content)) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    
    // Test Case 4: Empty filename (should fail)
    const char* file4 = "";
    printf("\nTest 4: Writing to empty filename\n");
    if (createFileAndWrite(file4, content)) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }
    
    // Test Case 5: Writing empty content
    const char* file5 = "c_test5.log";
    printf("\nTest 5: Writing an empty string to '%s'\n", file5);
    if (createFileAndWrite(file5, "")) {
        printf("Success.\n");
    } else {
        printf("Failed.\n");
    }

    return 0;
}