#include <stdio.h>
#include <stdbool.h> // For bool type
#include <fcntl.h>   // For O_CREAT, O_WRONLY, O_TRUNC, O_EXCL
#include <sys/stat.h> // For mode constants like S_IRUSR
#include <unistd.h>  // For open(), close(), write(), unlink()
#include <string.h>  // For strerror()
#include <errno.h>   // For errno

// Note: This code uses POSIX-specific functions (open, close, unlink).
// It will work on Linux, macOS, and other UNIX-like systems.
// For Windows, you may need to include <io.h> and use functions like _open(), _close().

/**
 * Opens a file with the specified POSIX flags and demonstrates the result.
 * @param pathname The path to the file.
 * @param flags A bitmask of the flags (e.g., O_CREAT | O_WRONLY).
 * @return true on success, false on failure.
 */
bool openFileWithFlags(const char* pathname, int flags) {
    // Permissions for new file: user read/write, group read, others read (0644)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    printf("Attempting to open '%s' with flags:", pathname);
    if (flags & O_CREAT) printf(" O_CREAT");
    if (flags & O_WRONLY) printf(" O_WRONLY");
    if (flags & O_TRUNC) printf(" O_TRUNC");
    if (flags & O_EXCL) printf(" O_EXCL");
    printf("\n");

    int fd = open(pathname, flags, mode);

    if (fd == -1) {
        fprintf(stderr, "  -> Error: %s\n", strerror(errno));
        return false;
    }

    printf("  -> Success: File opened/created with fd = %d\n", fd);
    const char* data = "hello";
    if (write(fd, data, strlen(data)) == -1) {
        fprintf(stderr, "  -> Warning: Failed to write to file: %s\n", strerror(errno));
    }
    close(fd);
    return true;
}

int main() {
    const char* filename = "c_test_file.txt";

    // Ensure the file does not exist before starting
    unlink(filename);

    printf("--- C File Open Flags Demonstration (POSIX) ---\n");

    // Test 1: O_CREAT | O_WRONLY | O_EXCL on a new file
    printf("\n[Test 1: Exclusive Create on new file - Should Succeed]\n");
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_EXCL);

    // Test 2: O_CREAT | O_WRONLY | O_EXCL on an existing file
    printf("\n[Test 2: Exclusive Create on existing file - Should Fail]\n");
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_EXCL);

    // Test 3: O_CREAT | O_WRONLY on an existing file
    printf("\n[Test 3: Open for Writing (O_WRONLY) - Should Succeed]\n");
    openFileWithFlags(filename, O_CREAT | O_WRONLY);

    // Test 4: O_CREAT | O_WRONLY | O_TRUNC on an existing file
    printf("\n[Test 4: Open for Writing with Truncate (O_TRUNC) - Should Succeed]\n");
    openFileWithFlags(filename, O_CREAT | O_WRONLY | O_TRUNC);

    // Test 5: Cleanup
    printf("\n[Test 5: Cleanup]\n");
    if (unlink(filename) == 0) {
        printf("  -> Success: Deleted '%s'\n", filename);
    } else {
        fprintf(stderr, "  -> Error: Failed to delete '%s': %s\n", filename, strerror(errno));
    }

    return 0;
}