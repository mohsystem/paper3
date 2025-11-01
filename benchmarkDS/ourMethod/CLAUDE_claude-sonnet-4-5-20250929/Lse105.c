
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

/* Create a file with restrictive permissions (0600 - owner read/write only)
 * Uses open() with O_CREAT|O_EXCL for atomic creation and immediate permission setting
 */
int create_restrictive_file(const char* filename, const char* content) {
    /* Define restrictive permissions: owner read/write only (0600)
     * This prevents unauthorized access from other users or groups
     */
    mode_t restrictive_perms = S_IRUSR | S_IWUSR;  /* 0600 */
    
    /* Open file with O_CREAT|O_EXCL|O_WRONLY|O_TRUNC
     * O_EXCL ensures atomic creation (fails if exists)
     * O_NOFOLLOW prevents following symlinks (security measure)
     * Set permissions at creation time to avoid TOCTOU
     */
    int fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_NOFOLLOW, restrictive_perms);
    
    if (fd < 0) {
        if (errno == EEXIST) {
            /* File exists - remove and recreate for idempotent behavior */
            if (unlink(filename) != 0) {
                fprintf(stderr, "Failed to remove existing file: %s\\n", strerror(errno));
                return -1;
            }
            fd = open(filename, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC | O_NOFOLLOW, restrictive_perms);
            if (fd < 0) {
                fprintf(stderr, "Failed to create file after removal: %s\\n", strerror(errno));
                return -1;
            }
        } else {
            fprintf(stderr, "Failed to open file: %s\\n", strerror(errno));
            return -1;
        }
    }
    
    /* Write content to file */
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written < 0 || (size_t)bytes_written != content_len) {
        fprintf(stderr, "Failed to write content: %s\\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    /* Explicitly set permissions using fchmod on the open file descriptor
     * This ensures permissions are set correctly regardless of umask
     */
    if (fchmod(fd, restrictive_perms) != 0) {
        fprintf(stderr, "Failed to set permissions: %s\\n", strerror(errno));
        close(fd);
        return -1;
    }
    
    /* Close file descriptor */
    if (close(fd) != 0) {
        fprintf(stderr, "Failed to close file: %s\\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

int main(void) {
    /* Test cases */
    const char* test_files[] = {
        "secret.txt",
        "test1_secret.txt",
        "test2_secret.txt",
        "test3_secret.txt",
        "test4_secret.txt"
    };
    
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";
    
    size_t i;
    for (i = 0; i < 5; i++) {
        if (create_restrictive_file(test_files[i], content) == 0) {
            printf("Successfully created %s with restrictive permissions (0600)\\n", test_files[i]);
        } else {
            fprintf(stderr, "Failed to create %s\\n", test_files[i]);
        }
    }
    
    return 0;
}
