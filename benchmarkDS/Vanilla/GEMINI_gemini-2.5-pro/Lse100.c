#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>   // For open() and O_flags
#include <sys/stat.h>  // For S_I permissions
#include <unistd.h>  // For close()
#include <errno.h>   // For errno
#include <string.h>  // For strerror

// Note: This code uses POSIX-specific functions (open, S_IRUSR, etc.).
// It will compile and run on Linux, macOS, and other Unix-like systems.

/**
 * @brief Creates a file with owner-only read/write permissions.
 * This function is idempotent: it returns true if the file is successfully
 * created or if it already exists.
 *
 * @param filename The name of the file to create.
 * @return true on success (created or exists), false on error.
 */
bool createConfigFile(const char* filename) {
    // Define permissions: read and write for the owner (0600)
    mode_t permissions = S_IRUSR | S_IWUSR;

    // Attempt to create the file exclusively.
    // O_CREAT: create if it doesn't exist
    // O_WRONLY: open for writing
    // O_EXCL: fail if the file already exists (atomic check-and-create)
    int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL, permissions);

    if (fd != -1) {
        // Success, the file was created.
        close(fd);
        printf("Successfully created '%s' with owner read/write permissions.\n", filename);
        return true;
    }

    // If open failed, check the reason.
    if (errno == EEXIST) {
        // The file already exists, which we treat as a success.
        printf("File '%s' already exists.\n", filename);
        return true;
    }

    // Another error occurred.
    fprintf(stderr, "Error creating file '%s': %s\n", filename, strerror(errno));
    return false;
}

int main() {
    const char* fileName = "important_config";
    const char* otherFileName = "another_config.txt";

    printf("--- Test Case 1: Create file for the first time ---\n");
    createConfigFile(fileName);

    printf("\n--- Test Case 2: Try to create the same file again ---\n");
    createConfigFile(fileName);

    printf("\n--- Test Case 3: Delete the file ---\n");
    if (remove(fileName) == 0) {
        printf("Deleted '%s' for re-testing.\n", fileName);
    } else {
        // If file doesn't exist, remove fails. That's okay in this context.
    }

    printf("\n--- Test Case 4: Create the file again after deletion ---\n");
    createConfigFile(fileName);

    printf("\n--- Test Case 5: Create a different file ---\n");
    createConfigFile(otherFileName);

    // Cleanup
    printf("\n--- Cleanup ---\n");
    remove(fileName);
    remove(otherFileName);
    printf("Cleanup complete.\n");

    return 0;
}