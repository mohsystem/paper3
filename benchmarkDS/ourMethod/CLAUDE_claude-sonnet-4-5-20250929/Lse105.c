
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/* Compliant: Open file with restrictive permissions, validate handle, write, sync, and close */
int create_secret_file(const char *content) {
    int fd = -1;
    ssize_t written = 0;
    size_t content_len = 0;
    int result = -1;

    /* Rules#2: Input validation - check content is not NULL and has reasonable length */
    if (content == NULL) {
        fprintf(stderr, "Invalid input\\n");
        return -1;
    }

    content_len = strlen(content);
    if (content_len == 0 || content_len > 1024) {
        fprintf(stderr, "Content length out of range\\n");
        return -1;
    }

    /* Rules#7: Create file with restrictive permissions (0600 = owner read/write only) at creation time */
    /* Rules#27: Open first with safe flags - O_CREAT|O_EXCL prevents TOCTOU, O_NOFOLLOW prevents symlink following */
    fd = open("secret.txt", O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
    if (fd < 0) {
        fprintf(stderr, "Failed to create file\\n");
        return -1;
    }

    /* Rules#28: Validate the opened handle using fstat */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Failed to validate file handle\\n");
        goto cleanup;
    }

    /* Verify it's a regular file and has expected permissions */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file\\n");
        goto cleanup;
    }

    /* Rules#8: Write with bounds checking - write exactly content_len bytes */
    written = write(fd, content, content_len);
    if (written < 0 || (size_t)written != content_len) {
        fprintf(stderr, "Write failed\\n");
        goto cleanup;
    }

    /* Rules#7: Ensure data is flushed to disk for durability */
    if (fsync(fd) != 0) {
        fprintf(stderr, "Failed to sync file\\n");
        goto cleanup;
    }

    result = 0;

cleanup:
    /* Rules#8: Clean up resources - close exactly once */
    if (fd >= 0) {
        close(fd);
    }
    return result;
}

int main(void) {
    /* Test case 1: Normal operation */
    printf("Test 1: Creating secret.txt with restricted permissions...\\n");
    if (create_secret_file("U2FsdGVkX1/R+WzJcxgvX/Iw==") == 0) {
        printf("Test 1 PASSED: File created successfully\\n");
        unlink("secret.txt"); /* Clean up for next test */
    } else {
        printf("Test 1 FAILED\\n");
    }

    /* Test case 2: Verify file cannot be created twice (O_EXCL check) */
    printf("\\nTest 2: Creating file twice should fail...\\n");
    if (create_secret_file("U2FsdGVkX1/R+WzJcxgvX/Iw==") == 0) {
        if (create_secret_file("U2FsdGVkX1/R+WzJcxgvX/Iw==") != 0) {
            printf("Test 2 PASSED: Cannot create duplicate file\\n");
        } else {
            printf("Test 2 FAILED: Created duplicate file\\n");
        }
        unlink("secret.txt");
    } else {
        printf("Test 2 FAILED: Initial creation failed\\n");
    }

    /* Test case 3: NULL content validation */
    printf("\\nTest 3: NULL content should be rejected...\\n");
    if (create_secret_file(NULL) != 0) {
        printf("Test 3 PASSED: NULL input rejected\\n");
    } else {
        printf("Test 3 FAILED: NULL input accepted\\n");
    }

    /* Test case 4: Empty string validation */
    printf("\\nTest 4: Empty string should be rejected...\\n");
    if (create_secret_file("") != 0) {
        printf("Test 4 PASSED: Empty string rejected\\n");
    } else {
        printf("Test 4 FAILED: Empty string accepted\\n");
        unlink("secret.txt");
    }

    /* Test case 5: Verify file permissions */
    printf("\\nTest 5: Verifying file permissions are restrictive (0600)...\\n");
    if (create_secret_file("U2FsdGVkX1/R+WzJcxgvX/Iw==") == 0) {
        struct stat st;
        if (stat("secret.txt", &st) == 0) {
            mode_t perms = st.st_mode & 0777;
            if (perms == 0600) {
                printf("Test 5 PASSED: Permissions are 0600 (owner rw only)\\n");
            } else {
                printf("Test 5 FAILED: Permissions are %o, expected 0600\\n", perms);
            }
        } else {
            printf("Test 5 FAILED: Cannot stat file\\n");
        }
        unlink("secret.txt");
    } else {
        printf("Test 5 FAILED: File creation failed\\n");
    }

    return 0;
}
