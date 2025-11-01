#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

// POSIX headers for file control
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Atomically creates a file with secure permissions (0600).
 *
 * This function uses the POSIX open() system call with flags that ensure
 * the file creation and permission setting is a single, atomic operation.
 * This is the secure way to create files to avoid race conditions.
 *
 * @param filePath The path of the file to create.
 * @return true if the file was created successfully, false otherwise.
 */
bool createSecureFile(const char* filePath) {
    // Mode for owner read/write (rw-------), equivalent to octal 0600.
    mode_t mode = S_IRUSR | S_IWUSR;

    // Flags for atomic creation:
    // O_WRONLY: Open for writing only.
    // O_CREAT: Create the file if it does not exist.
    // O_EXCL: With O_CREAT, fail if the file already exists. This prevents race conditions.
    int flags = O_WRONLY | O_CREAT | O_EXCL;

    // The open() system call is atomic.
    int fd = open(filePath, flags, mode);

    if (fd == -1) {
        // An error occurred. The global 'errno' variable holds the error code.
        fprintf(stderr, "Error creating file '%s': %s\n", filePath, strerror(errno));
        return false;
    }

    // File created successfully. Close the file descriptor immediately.
    close(fd);
    printf("Successfully created secure file: %s\n", filePath);
    return true;
}

int main() {
    const char* fileName1 = "important_config";
    const char* fileName2 = "another_config";
    const char* invalidPath = "non_existent_dir/important_config";
    const char* relativePath = "./temp_config";

    printf("--- Test Case 1: Create new file ---\n");
    createSecureFile(fileName1);

    printf("\n--- Test Case 2: Attempt to create existing file ---\n");
    createSecureFile(fileName1);

    printf("\n--- Test Case 3: Create a different new file ---\n");
    createSecureFile(fileName2);

    printf("\n--- Test Case 4: Attempt to create file in non-existent directory ---\n");
    createSecureFile(invalidPath);
    
    printf("\n--- Test Case 5: Create a file with a relative path ---\n");
    createSecureFile(relativePath);

    // Cleanup
    printf("\n--- Cleaning up created files ---\n");
    remove(fileName1);
    remove(fileName2);
    remove(relativePath);
    printf("Cleanup complete.\n");

    return 0;
}