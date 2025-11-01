#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/**
 * @brief Demonstrates the POSIX open() call with various flags.
 * 
 * @param path The path to the file.
 * @param flags The bitmask of flags for open().
 * @param description A description of the test case.
 */
void demonstrate_open(const char* path, int flags, const char* description) {
    printf("--- Test: %s ---\n", description);
    printf("Attempting to open '%s'...\n", path);

    mode_t mode = S_IRUSR | S_IWUSR; // 0600 permissions
    int fd;

    // The mode argument is only used when O_CREAT is specified.
    if ((flags & O_CREAT) != 0) {
        fd = open(path, flags, mode);
    } else {
        fd = open(path, flags);
    }

    if (fd == -1) {
        fprintf(stderr, "Failed as expected: ");
        perror(NULL);
    } else {
        printf("Success: File opened with descriptor %d.\n", fd);
        
        if (strstr(description, "Write data") != NULL) {
            const char* content = "hello world";
            ssize_t bytes_written = write(fd, content, strlen(content));
            if (bytes_written == -1) {
                perror("Failed to write to file");
            } else {
                printf("Successfully wrote to file.\n");
            }
        }
        
        if (close(fd) == -1) {
            perror("Failed to close file descriptor");
        }
    }
    printf("\n");
}

int main() {
    const char* test_file = "c_testfile.txt";
    const char* non_existent_file = "c_non_existent.txt";

    // Initial cleanup to ensure a clean state for tests
    remove(test_file);
    remove(non_existent_file);

    // Test 1: O_WRONLY | O_CREAT | O_EXCL - Atomically create a new file. Fails if it exists.
    puts("1) Demonstrating O_EXCL (exclusive creation)");
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_EXCL, "O_WRONLY | O_CREAT | O_EXCL (exclusive create)");

    // Test 2: Try to create it again with O_EXCL. Must fail.
    puts("2) Demonstrating O_EXCL on an existing file");
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_EXCL, "O_WRONLY | O_CREAT | O_EXCL (on existing file)");

    // Test 3: Write some data to the file to demonstrate truncation later.
    puts("3) Writing data to the file");
    demonstrate_open(test_file, O_WRONLY | O_TRUNC, "Write data to existing file");

    // Test 4: O_WRONLY | O_CREAT | O_TRUNC - Create if not exists, or truncate if it does.
    puts("4) Demonstrating O_TRUNC on an existing file");
    demonstrate_open(test_file, O_WRONLY | O_CREAT | O_TRUNC, "O_WRONLY | O_CREAT | O_TRUNC (on existing file)");

    // Test 5: O_WRONLY on a non-existent file (without O_CREAT). Must fail.
    puts("5) Demonstrating opening a non-existent file without O_CREAT");
    demonstrate_open(non_existent_file, O_WRONLY, "O_WRONLY (on non-existent file)");

    // Final cleanup
    remove(test_file);

    return 0;
}