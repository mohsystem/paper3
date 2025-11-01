#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

// This code is intended for POSIX-compliant systems (e.g., Linux, macOS).

/**
 * Creates a file with the given name and writes the specified content to it securely.
 * This function uses a temporary file and an atomic rename operation to prevent
 * race conditions and ensure the file is written completely or not at all.
 *
 * @param file_name The name of the file to create.
 * @param content The content to write to the file.
 * @return 0 on success, -1 on failure.
 */
int write_important_config(const char *file_name, const char *content) {
    int fd = -1;
    int dir_fd = -1;
    char *temp_template = NULL;
    int result = -1;

    // Allocate memory for the temporary file name template
    size_t template_len = strlen(file_name) + 8; // for ".XXXXXX" and '\0'
    temp_template = malloc(template_len);
    if (!temp_template) {
        perror("malloc");
        return -1;
    }
    snprintf(temp_template, template_len, "%s.XXXXXX", file_name);
    
    // Create and open a temporary file securely with 0600 permissions.
    fd = mkstemp(temp_template);
    if (fd == -1) {
        perror("mkstemp");
        goto cleanup;
    }

    size_t content_len = strlen(content);
    if (write(fd, content, content_len) != (ssize_t)content_len) {
        perror("write");
        goto cleanup;
    }

    if (fsync(fd) == -1) {
        perror("fsync");
        goto cleanup;
    }

    if (close(fd) == -1) {
        perror("close temp fd");
        fd = -1; // Mark as closed
        goto cleanup;
    }
    fd = -1; // Mark as closed, ownership transfers to rename/unlink

    if (rename(temp_template, file_name) == -1) {
        perror("rename");
        goto cleanup;
    }

    // Sync the parent directory to ensure the rename is persisted.
    dir_fd = open(".", O_RDONLY | O_CLOEXEC);
    if (dir_fd == -1) {
        fprintf(stderr, "Warning: could not open directory to sync: %s\n", strerror(errno));
    } else {
        if (fsync(dir_fd) == -1) {
            fprintf(stderr, "Warning: could not sync directory: %s\n", strerror(errno));
        }
        close(dir_fd);
    }

    result = 0; // Success!

cleanup:
    if (result != 0) {
        // An error occurred; remove the temp file if it was created.
        if (temp_template != NULL) {
            unlink(temp_template);
        }
    }
    if (fd != -1) {
        close(fd);
    }
    free(temp_template);
    return result;
}

int main(void) {
    const char* file_name = "important_config";
    const char* content = "important_config";

    for (int i = 1; i <= 5; ++i) {
        printf("Test Case %d:\n", i);
        if (write_important_config(file_name, content) == 0) {
            printf("Successfully wrote to %s\n", file_name);
        } else {
            printf("Failed to write to %s\n", file_name);
        }
        printf("--------------------\n");
    }
    return 0;
}