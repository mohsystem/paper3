
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int createFileExcl(const char* filename) {
    // O_CREAT: create if doesn't exist
    // O_EXCL: fail if exists
    // O_WRONLY: write only
    // O_TRUNC: truncate to 0 bytes
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

int createFileTrunc(const char* filename) {
    // Without O_EXCL - will truncate existing files
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd == -1) {
        return -1;
    }
    close(fd);
    return 0;
}

int main() {
    // Test case 1: Create new file with O_EXCL
    printf("Test 1 - Create new file: %d\\n", createFileExcl("test1.txt"));
    
    // Test case 2: Try to create existing file with O_EXCL (should fail)
    printf("Test 2 - Create existing file with O_EXCL: %d\\n", createFileExcl("test1.txt"));
    
    // Test case 3: Create/truncate file without O_EXCL
    printf("Test 3 - Create/truncate file: %d\\n", createFileTrunc("test2.txt"));
    
    // Test case 4: Truncate existing file
    printf("Test 4 - Truncate existing file: %d\\n", createFileTrunc("test2.txt"));
    
    // Test case 5: Create file in valid directory
    printf("Test 5 - Create another file: %d\\n", createFileExcl("test3.txt"));
    
    // Cleanup
    unlink("test1.txt");
    unlink("test2.txt");
    unlink("test3.txt");
    
    return 0;
}
