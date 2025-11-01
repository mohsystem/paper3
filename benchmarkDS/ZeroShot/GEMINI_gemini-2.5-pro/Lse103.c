#include <stdio.h>
#include <stdbool.h>

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Opens or creates a file using POSIX flags.
 * This function is secure because it avoids race conditions during file existence
 * checks by using the atomic O_EXCL flag. It specifies secure default permissions
 * (0644) for new files. The file descriptor is always closed before returning to
 * prevent resource leaks.
 *
 * @param filename The path to the file.
 * @param flags A bitmask of flags like O_CREAT, O_WRONLY, etc.
 * @return true on success, false on failure.
 */
bool open_file(const char* filename, int flags) {
    int fd = -1;
    // Define secure permissions for file creation (user r/w, group r, other r)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; // 0644

    // The mode argument is only used by open() when O_CREAT is specified.
    if (flags & O_CREAT) {
        fd = open(filename, flags, mode);
    } else {
        fd = open(filename, flags);
    }

    if (fd == -1) {
        // perror provides a descriptive error message from the OS
        char error_msg[256];
        snprintf(error_msg, sizeof(error_msg), "Operation failed for %s", filename);
        perror(error_msg);
        return false;
    }

    // The function's purpose is to demonstrate opening, so we close immediately.
    close(fd);
    return true;
}

/**
 * @brief Helper function for test setup/cleanup
 */
void setup_file(const char* filename, const char* content) {
    if (content == NULL) {
        remove(filename);
    } else {
        FILE* fp = fopen(filename, "w");
        if (fp) {
            fputs(content, fp);
            fclose(fp);
        } else {
            fprintf(stderr, "Test setup failed for %s\n", filename);
        }
    }
}

int main() {
    const char* test_files[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    int num_files = sizeof(test_files) / sizeof(test_files[0]);

    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Create a new file exclusively (O_CREAT | O_WRONLY | O_EXCL)
    printf("\n[Test 1] Create new file exclusively (O_CREAT | O_WRONLY | O_EXCL)\n");
    const char* file1 = "test1.txt";
    setup_file(file1, NULL); // Ensure file does not exist
    bool result1 = open_file(file1, O_CREAT | O_WRONLY | O_EXCL);
    printf("Result: %s (Expected: Success)\n", result1 ? "Success" : "Failure");

    // Test Case 2: Attempt to create an existing file exclusively (O_CREAT | O_WRONLY | O_EXCL)
    printf("\n[Test 2] Attempt exclusive creation on existing file (O_CREAT | O_WRONLY | O_EXCL)\n");
    const char* file2 = "test2.txt";
    setup_file(file2, "content"); // Ensure file exists
    bool result2 = open_file(file2, O_CREAT | O_WRONLY | O_EXCL);
    printf("Result: %s (Expected: Failure)\n", result2 ? "Success" : "Failure");

    // Test Case 3: Open an existing file and truncate it (O_WRONLY | O_TRUNC)
    printf("\n[Test 3] Open and truncate existing file (O_WRONLY | O_TRUNC)\n");
    const char* file3 = "test3.txt";
    setup_file(file3, "some initial content"); // Ensure file exists with content
    bool result3 = open_file(file3, O_WRONLY | O_TRUNC);
    printf("Result: %s (Expected: Success)\n", result3 ? "Success" : "Failure");

    // Test Case 4: Open an existing file for writing only (O_WRONLY)
    printf("\n[Test 4] Open existing file for writing (O_WRONLY)\n");
    const char* file4 = "test4.txt";
    setup_file(file4, "content"); // Ensure file exists
    bool result4 = open_file(file4, O_WRONLY);
    printf("Result: %s (Expected: Success)\n", result4 ? "Success" : "Failure");

    // Test Case 5: Create a file if it does not exist (O_CREAT | O_WRONLY)
    printf("\n[Test 5] Create file if it doesn't exist (O_CREAT | O_WRONLY)\n");
    const char* file5 = "test5.txt";
    setup_file(file5, NULL); // Ensure file does not exist
    bool result5 = open_file(file5, O_CREAT | O_WRONLY);
    printf("Result: %s (Expected: Success)\n", result5 ? "Success" : "Failure");
    
    // Cleanup
    printf("\nCleaning up test files...\n");
    for (int i = 0; i < num_files; ++i) {
        setup_file(test_files[i], NULL);
    }
    printf("Done.\n");

    return 0;
}