
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Security: Maximum file size to prevent DoS - 10MB */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
/* Security: Maximum filename length to prevent buffer overflow */
#define MAX_FILENAME_LEN 255
/* Security: Buffer size for safe I/O operations */
#define BUFFER_SIZE 8192

/* Security: Explicit memset that cannot be optimized away */
void secure_memset(void *ptr, int value, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) {
        *p++ = (unsigned char)value;
    }
}

/* Security: Validate filename to prevent path traversal and injection attacks
 * Rules#2: Input validation with allow list approach */
int validate_filename(const char *filename) {
    size_t len;
    size_t i;
    int has_pdf_ext = 0;
    
    if (filename == NULL) {
        return 0;
    }
    
    len = strnlen(filename, MAX_FILENAME_LEN + 1);
    
    /* Security: Reject if filename exceeds maximum length */
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return 0;
    }
    
    /* Security: Check for path traversal attempts */
    if (strstr(filename, "..") != NULL || 
        strstr(filename, "/") != NULL || 
        strstr(filename, "\\\") != NULL) {\n        return 0;\n    }\n    \n    /* Security: Validate allowed characters (alphanumeric, dash, underscore, dot) */\n    for (i = 0; i < len; i++) {\n        char c = filename[i];\n        if (!((c >= 'a' && c <= 'z') || \n              (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || \n              c == '-' || c == '_' || c == '.')) {\n            return 0;\n        }\n    }\n    \n    /* Security: Verify .pdf extension */\n    if (len > 4 && strcmp(filename + len - 4, ".pdf") == 0) {\n        has_pdf_ext = 1;\n    }\n    \n    return has_pdf_ext;\n}\n\n/* Security: Safe file upload with TOCTOU prevention\n * Rules#7: Open first, validate handle, operate only on handle */\nint upload_pdf_file(const char *filename, const unsigned char *file_data, size_t file_size) {\n    int dirfd = -1;\n    int fd = -1;\n    ssize_t written = 0;\n    ssize_t total_written = 0;\n    struct stat st;\n    char safe_path[PATH_MAX];\n    int result = -1;\n    char temp_filename[MAX_FILENAME_LEN + 16];\n    \n    /* Security: Rules#2 - Validate all inputs */\n    if (filename == NULL || file_data == NULL) {\n        fprintf(stderr, "Error: Invalid parameters\
");\n        return -1;\n    }\n    \n    /* Security: Rules#36 - Check file size limits to prevent DoS */\n    if (file_size == 0 || file_size > MAX_FILE_SIZE) {\n        fprintf(stderr, "Error: Invalid file size\
");\n        return -1;\n    }\n    \n    /* Security: Rules#2 - Validate filename to prevent path traversal */\n    if (!validate_filename(filename)) {\n        fprintf(stderr, "Error: Invalid filename\
");\n        return -1;\n    }\n    \n    /* Security: Rules#7 - Open uploads directory with O_DIRECTORY to anchor operations */\n    dirfd = open("uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC);\n    if (dirfd == -1) {\n        /* Create directory if it doesn't exist with restrictive permissions */\n        if (mkdir("uploads", 0700) != 0 && errno != EEXIST) {\n            fprintf(stderr, "Error: Cannot create uploads directory\
");\n            return -1;\n        }\n        dirfd = open("uploads", O_RDONLY | O_DIRECTORY | O_CLOEXEC);\n        if (dirfd == -1) {\n            fprintf(stderr, "Error: Cannot open uploads directory\
");\n            return -1;\n        }\n    }\n    \n    /* Security: Rules#52 - Write to temporary file first for atomic operation */\n    /* Security: Rules#8 - Check snprintf return value and buffer size */\n    if (snprintf(temp_filename, sizeof(temp_filename), ".tmp_%s", filename) >= (int)sizeof(temp_filename)) {\n        fprintf(stderr, "Error: Filename too long\
");\n        close(dirfd);\n        return -1;\n    }\n    \n    /* Security: Rules#7 - Use openat with O_NOFOLLOW to prevent symlink attacks\n     * Rules#50 - Set restrictive permissions (0600) at creation time */\n    fd = openat(dirfd, temp_filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);\n    if (fd == -1) {\n        fprintf(stderr, "Error: Cannot create temporary file\
");\n        close(dirfd);\n        return -1;\n    }\n    \n    /* Security: Rules#48 - Validate opened file descriptor with fstat */\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Error: Cannot stat file\
");\n        goto cleanup;\n    }\n    \n    /* Security: Verify it's a regular file */\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Not a regular file\
");\n        goto cleanup;\n    }\n    \n    /* Security: Rules#8 - Write data with bounds checking and error handling */\n    total_written = 0;\n    while (total_written < (ssize_t)file_size) {\n        size_t to_write = file_size - total_written;\n        if (to_write > BUFFER_SIZE) {\n            to_write = BUFFER_SIZE;\n        }\n        \n        written = write(fd, file_data + total_written, to_write);\n        if (written <= 0) {\n            if (written == -1 && errno == EINTR) {\n                continue;\n            }\n            fprintf(stderr, "Error: Write failed\
");\n            goto cleanup;\n        }\n        \n        /* Security: Rules#35 - Check for overflow before addition */\n        if (total_written > SSIZE_MAX - written) {\n            fprintf(stderr, "Error: Integer overflow in write\
");\n            goto cleanup;\n        }\n        total_written += written;\n    }\n    \n    /* Security: Rules#52 - Flush and sync before rename for durability */\n    if (fsync(fd) != 0) {\n        fprintf(stderr, "Error: fsync failed\
");\n        goto cleanup;\n    }\n    \n    /* Close the file descriptor before rename */\n    close(fd);\n    fd = -1;\n    \n    /* Security: Rules#52 - Atomic rename to final destination */\n    if (renameat(dirfd, temp_filename, dirfd, filename) != 0) {\n        fprintf(stderr, "Error: Rename failed\
");\n        unlinkat(dirfd, temp_filename, 0);\n        close(dirfd);\n        return -1;\n    }\n    \n    /* Security: Sync directory for durability */\n    fsync(dirfd);\n    \n    result = 0;\n    \ncleanup:\n    /* Security: Rules#8 - Proper cleanup and error handling */\n    if (fd != -1) {\n        close(fd);\n        /* Remove temporary file on failure */\n        unlinkat(dirfd, temp_filename, 0);\n    }\n    if (dirfd != -1) {\n        close(dirfd);\n    }\n    \n    return result;\n}\n\n/* Test cases */\nint main(void) {\n    int test_result;\n    unsigned char test_data[1024];\n    size_t i;\n    \n    /* Initialize test data */\n    for (i = 0; i < sizeof(test_data); i++) {\n        test_data[i] = (unsigned char)(i % 256);\n    }\n    \n    printf("Test 1: Valid PDF upload\
");\n    test_result = upload_pdf_file("test1.pdf", test_data, sizeof(test_data));\n    printf("Result: %s\
\
", test_result == 0 ? "SUCCESS" : "FAILED");\n    \n    printf("Test 2: Path traversal attempt (should fail)\
");\n    test_result = upload_pdf_file("../etc/passwd.pdf", test_data, sizeof(test_data));\n    printf("Result: %s\
\
", test_result != 0 ? "SUCCESS (blocked)" : "FAILED");\n    \n    printf("Test 3: Invalid extension (should fail)\
");\n    test_result = upload_pdf_file("test.txt", test_data, sizeof(test_data));\n    printf("Result: %s\
\
", test_result != 0 ? "SUCCESS (blocked)" : "FAILED");\n    \n    printf("Test 4: Null filename (should fail)\
");\n    test_result = upload_pdf_file(NULL, test_data, sizeof(test_data));\n    printf("Result: %s\
\
", test_result != 0 ? "SUCCESS (blocked)" : "FAILED");\n    \n    printf("Test 5: Oversized file (should fail)\
");\n    test_result = upload_pdf_file("large.pdf", test_data, MAX_FILE_SIZE + 1);\n    printf("Result: %s\
\
", test_result != 0 ? "SUCCESS (blocked)" : "FAILED");
    
    return 0;
}
