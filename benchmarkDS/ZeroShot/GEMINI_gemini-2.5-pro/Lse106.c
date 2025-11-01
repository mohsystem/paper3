#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

// The secure file creation functions used here (mkstemp, chmod) are POSIX-specific.
// This code will not compile on a standard Windows environment without a POSIX layer like Cygwin or WSL.
#if defined(__unix__) || defined(__APPLE__)

/**
 * @brief Creates a file, writes secret information to it, and changes its permissions
 * to be read-only for the owner.
 *
 * @param secretInfo The string content to write to the file.
 * @return A dynamically allocated string with the filename on success, NULL on failure.
 *         The caller is responsible for freeing the returned string.
 */
char* createSecretFile(const char* secretInfo) {
    // 1. Create a template for a secure temporary file.
    // The template must be a mutable char array for mkstemp.
    char template_path[] = "/tmp/secret_XXXXXX";
    
    // mkstemp securely creates a unique file and returns a file descriptor.
    int fd = mkstemp(template_path);
    if (fd == -1) {
        perror("mkstemp failed");
        return NULL;
    }

    // 2. Write the secret info to the file.
    ssize_t info_len = strlen(secretInfo);
    if (write(fd, secretInfo, info_len) != info_len) {
        perror("write failed");
        close(fd);
        unlink(template_path); // Clean up (delete) the file on failure
        return NULL;
    }

    close(fd);

    // 3. Change permissions to read-only for the owner (0400).
    // S_IRUSR is the constant for owner-read permission.
    if (chmod(template_path, S_IRUSR) == -1) {
        perror("chmod failed");
        unlink(template_path); // Clean up the file on failure
        return NULL;
    }

    // 4. Return the file path. strdup allocates new memory for the path.
    char* result_path = strdup(template_path);
    if (result_path == NULL) {
        perror("strdup failed");
        unlink(template_path); // Clean up the file on failure
        return NULL;
    }

    return result_path;
}

int main() {
    printf("Running C Test Cases:\n");
    for (int i = 1; i <= 5; i++) {
        char secret_buffer[100];
        snprintf(secret_buffer, sizeof(secret_buffer), "C super secret key #%d is CK%d", i, i * 321);

        char* fileName = createSecretFile(secret_buffer);
        if (fileName != NULL) {
            printf("Test Case %d: Secret file created at: %s\n", i, fileName);
            // The caller is responsible for freeing the memory allocated by strdup.
            free(fileName);
        } else {
            fprintf(stderr, "Test Case %d: Failed to create secret file.\n", i);
        }
    }
    return 0;
}

#else
// Provide a main function for non-POSIX systems like Windows to allow compilation.
int main() {
    printf("Running C Test Cases:\n");
    fprintf(stderr, "This secure file creation example is designed for POSIX-compliant systems (Linux, macOS) and will not run on this platform.\n");
    return 1;
}
#endif