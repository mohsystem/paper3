#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// POSIX headers for file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * @brief Creates a file with secure permissions (0600: owner read/write only).
 *
 * This function uses POSIX open() with O_CREAT and O_EXCL flags to ensure
 * atomic file creation. It will fail if the file already exists, preventing
 * race conditions and overwriting existing files (including symlinks).
 * O_NOFOLLOW is used where available to prevent following symbolic links.
 *
 * @param filename The name of the file to create.
 * @return true if the file was created successfully, false otherwise.
 */
bool create_secure_file(const char* filename) {
    // O_CREAT: Create the file if it does not exist.
    // O_EXCL:  When used with O_CREAT, the call will fail if the file already exists.
    // O_WRONLY: Open for writing only.
    int flags = O_CREAT | O_EXCL | O_WRONLY;

// O_NOFOLLOW is a security enhancement but not in all POSIX standards.
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    // S_IRUSR | S_IWUSR corresponds to mode 0600 (read/write for owner only).
    mode_t mode = S_IRUSR | S_IWUSR;

    int fd = open(filename, flags, mode);

    if (fd == -1) {
        // Construct a more descriptive error message
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Error creating file %s", filename);
        perror(error_msg);
        return false;
    }

    // The file has been created successfully. We can now close the descriptor.
    close(fd);
    printf("Successfully created file: %s\n", filename);
    return true;
}

int main() {
    const char* test_files[] = {
        "important_config_1.txt",
        "important_config_2.cfg",
        "another_secure_file.dat",
        "test_config.ini",
        "user_settings.json"
    };
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);

    printf("--- Running 5 Test Cases ---\n");

    for (int i = 0; i < num_tests; i++) {
        const char* filename = test_files[i];
        printf("\n--- Test Case %d: %s ---\n", i + 1, filename);
        
        // 1. Attempt to create the file (should succeed)
        printf("Attempt 1: Creating file.\n");
        create_secure_file(filename);

        // 2. Attempt to create the same file again (should fail)
        printf("Attempt 2: Creating the same file again.\n");
        create_secure_file(filename);

        // 3. Clean up the created file
        if (remove(filename) == 0) {
            printf("Cleaned up file: %s\n", filename);
        } else {
            fprintf(stderr, "Failed to clean up file: %s\n", filename);
        }
    }

    return 0;
}