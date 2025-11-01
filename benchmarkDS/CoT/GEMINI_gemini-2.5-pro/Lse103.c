#include <stdio.h>
#include <fcntl.h>    // For open() and O_* flags
#include <unistd.h>   // For write(), close()
#include <sys/stat.h> // For mode constants like S_IRUSR
#include <errno.h>    // For errno
#include <string.h>   // For strerror()

/**
 * Creates or opens a file using the open() system call with specified flags.
 * O_CREAT: Create the file if it doesn't exist.
 * O_TRUNC: Truncate the file to 0 bytes if it already exists.
 * O_WRONLY: Open the file for writing only.
 * O_EXCL: When used with O_CREAT, fail if the file already exists.
 *
 * In a real-world application, sanitize the 'filePath' parameter to
 * prevent path traversal vulnerabilities.
 * 
 * @param filePath The path to the file.
 * @param flags A bitmask of flags for open().
 * @return 1 on success, 0 on failure.
 */
int demonstrate_open(const char* filePath, int flags) {
    // The mode is only used when O_CREAT is specified.
    // S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH corresponds to 0644 permissions.
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    int fd = open(filePath, flags, mode);

    if (fd == -1) {
        fprintf(stderr, "Error: Failed to open '%s': %s\n", filePath, strerror(errno));
        return 0;
    }

    printf("Success: Opened/created '%s'. File descriptor: %d\n", filePath, fd);
    
    // Write some content to demonstrate writability.
    const char* text = "C test content.\n";
    if (write(fd, text, strlen(text)) == -1) {
        fprintf(stderr, "Error writing to '%s': %s\n", filePath, strerror(errno));
    }
    
    // Always close the file descriptor to prevent resource leaks.
    if (close(fd) == -1) {
        fprintf(stderr, "Error: Failed to close '%s': %s\n", filePath, strerror(errno));
        return 0;
    }
    
    return 1;
}

int main() {
    printf("--- C Test Cases ---\n");

    const char* file1 = "test_c_1.txt";
    const char* file2 = "test_c_2.txt";

    // TC1: O_CREAT | O_WRONLY | O_TRUNC (Create new file, truncate if exists)
    printf("\n[TC1] Creating new file with O_CREAT | O_WRONLY | O_TRUNC...\n");
    demonstrate_open(file1, O_CREAT | O_WRONLY | O_TRUNC);
    
    // TC2: O_CREAT | O_WRONLY | O_TRUNC on an existing file (should truncate).
    printf("\n[TC2] Opening existing file with O_TRUNC (should truncate)...\n");
    demonstrate_open(file1, O_CREAT | O_WRONLY | O_TRUNC);

    // TC3: O_CREAT | O_WRONLY | O_EXCL (Create new file exclusively)
    printf("\n[TC3] Creating new file with O_EXCL...\n");
    demonstrate_open(file2, O_CREAT | O_WRONLY | O_EXCL);

    // TC4: O_CREAT | O_WRONLY | O_EXCL on an existing file (should fail).
    printf("\n[TC4] Attempting to create existing file with O_EXCL (should fail)...\n");
    demonstrate_open(file2, O_CREAT | O_WRONLY | O_EXCL);
    
    // TC5: O_WRONLY (Open existing file for writing without truncating)
    printf("\n[TC5] Opening existing file with O_WRONLY (no truncate)...\n");
    demonstrate_open(file1, O_WRONLY);

    // Cleanup
    printf("\n--- Cleanup ---\n");
    remove(file1);
    remove(file2);
    printf("Cleanup complete.\n");

    return 0;
}