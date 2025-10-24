#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // For mkstemp, getpid, unlink, write, close
#include <time.h>   // For time
#include <errno.h>

// Note: This implementation uses the POSIX-specific mkstemp() function for
// creating secure temporary files. It is not portable to non-POSIX systems
// like Windows without using different, platform-specific APIs.

/**
 * Generates a random alphanumeric string of a given length.
 * Note: Uses rand(), which is not cryptographically secure. For security-critical
 * applications, use a better source of randomness like reading from /dev/urandom.
 * @param length The length of the alphanumeric part.
 * @return A dynamically allocated string which must be freed by the caller.
 */
char* generateRandomString(int length) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // Allocate space for the random part, the UTF-8 suffix, and null terminator.
    char* random_string = malloc(length + 5); 
    if (!random_string) {
        return NULL;
    }

    for (int i = 0; i < length; ++i) {
        int key = rand() % (int)(sizeof(charset) - 1);
        random_string[i] = charset[key];
    }
    
    // Add a unicode character (Euro sign) to demonstrate UTF-8
    // € in UTF-8 is 3 bytes: 0xE2 0x82 0xAC
    strcpy(random_string + length, "-\xE2\x82\xAC");

    return random_string;
}

/**
 * Creates a secure temporary file, writes a random UTF-8 string to it, and
 * returns the file path.
 * @return A dynamically allocated string containing the file path, which the
 *         caller must free. Returns NULL on failure.
 */
char* createTempFileWithRandomUnicode() {
    // 1. Prepare a template for the temporary file path.
    // mkstemp requires a template ending in XXXXXX.
    // Use P_tmpdir if defined, otherwise fallback to /tmp.
    char template_path[256];
    const char *tmpdir = getenv("TMPDIR");
    if (tmpdir == NULL) {
        tmpdir = P_tmpdir;
    }
    if (tmpdir == NULL) {
        tmpdir = "/tmp";
    }
    snprintf(template_path, sizeof(template_path), "%s/tempfile-XXXXXX", tmpdir);
    
    // 2. Securely create the temporary file.
    // mkstemp replaces XXXXXX with a unique string and creates the file with
    // secure permissions (0600). It returns an open file descriptor.
    int fd = mkstemp(template_path);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }

    // 3. Generate a random string.
    char* random_content = generateRandomString(32);
    if (!random_content) {
        fprintf(stderr, "Failed to generate random string\n");
        close(fd);
        unlink(template_path); // Clean up the empty file
        return NULL;
    }
    
    // 4. Write the string to the file.
    ssize_t bytes_written = write(fd, random_content, strlen(random_content));
    free(random_content); // Content is written, free it now.
    close(fd);

    if (bytes_written == -1) {
        perror("write failed");
        unlink(template_path); // Clean up the file on write error
        return NULL;
    }

    // 5. Return the path. strdup allocates new memory for the path.
    return strdup(template_path);
}

int main() {
    // Seed the random number generator.
    srand(time(NULL) ^ getpid());

    printf("Running 5 test cases for C...\n");
    char* file_paths[5];
    int created_count = 0;

    for (int i = 0; i < 5; i++) {
        char* file_path = createTempFileWithRandomUnicode();
        if (file_path) {
            printf("Test Case %d: Success. File created at: %s\n", i + 1, file_path);
            file_paths[created_count++] = file_path;
        } else {
            fprintf(stderr, "Test Case %d: Failed.\n", i + 1);
        }
    }

    // Cleanup
    if (created_count > 0) {
        printf("\nCleaning up temporary files...\n");
        for (int i = 0; i < created_count; i++) {
            if (unlink(file_paths[i]) == 0) {
                printf("Removed: %s\n", file_paths[i]);
            } else {
                perror("Failed to remove file");
            }
            free(file_paths[i]); // Free the memory allocated by strdup
        }
    }
    
    return 0;
}