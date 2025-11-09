
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#define UPLOAD_DIR "uploads"
#define MAX_FILENAME_LEN 255
#define MAX_PATH_LEN 4096
#define BUFFER_SIZE 8192
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB limit

// Secure memory clearing function
static void secure_zero(void *ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char *p = (volatile unsigned char *)ptr;
    while (len--) *p++ = 0;
}

// Validate filename: allow only alphanumeric, dash, underscore, and dot
// Reject path traversal attempts and hidden files
static int validate_filename(const char *filename) {
    if (filename == NULL || filename[0] == '\0') {
        return 0; // Empty filename
    }
    
    size_t len = strnlen(filename, MAX_FILENAME_LEN + 1);
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return 0; // Too long or empty
    }
    
    // Reject if starts with dot (hidden file) or contains path separators
    if (filename[0] == '.' || strchr(filename, '/') != NULL || strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    // Check for path traversal sequences\n    if (strstr(filename, "..") != NULL) {\n        return 0;\n    }\n    \n    // Validate characters: allow alphanumeric, dash, underscore, dot\n    for (size_t i = 0; i < len; i++) {\n        if (!isalnum((unsigned char)filename[i]) && \n            filename[i] != '-' && \n            filename[i] != '_' && \n            filename[i] != '.') {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n// Create upload directory securely\nstatic int create_upload_directory(void) {\n    struct stat st;\n    \n    // Try to create directory with restrictive permissions (0700)\n    if (mkdir(UPLOAD_DIR, 0700) == 0) {\n        return 0; // Successfully created\n    }\n    \n    if (errno != EEXIST) {\n        return -1; // Failed to create for reasons other than already exists\n    }\n    \n    // Directory exists - open and verify it's actually a directory
    int dirfd = open(UPLOAD_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        return -1; // Cannot open as directory or is a symlink
    }
    
    // Verify it's a directory using fstat on the opened descriptor\n    if (fstat(dirfd, &st) != 0) {\n        close(dirfd);\n        return -1;\n    }\n    \n    close(dirfd);\n    \n    if (!S_ISDIR(st.st_mode)) {\n        return -1; // Not a directory\n    }\n    \n    return 0;\n}\n\n// Upload file function with comprehensive security checks\nint upload_file(const char *source_path, const char *filename) {\n    // Initialize all pointers to NULL for safe cleanup\n    FILE *src = NULL;\n    FILE *dest = NULL;\n    unsigned char *buffer = NULL;\n    char *dest_path = NULL;\n    char *temp_path = NULL;\n    int result = -1;\n    struct stat src_stat;\n    struct stat dest_stat;\n    \n    // Validate input parameters\n    if (source_path == NULL || filename == NULL) {\n        fprintf(stderr, "Invalid parameters\\n");\n        return -1;\n    }\n    \n    // Validate filename against injection and traversal attacks\n    if (!validate_filename(filename)) {\n        fprintf(stderr, "Invalid filename\\n");\n        return -1;\n    }\n    \n    // Create upload directory securely (TOCTOU-safe)\n    if (create_upload_directory() != 0) {\n        fprintf(stderr, "Failed to create upload directory\\n");\n        return -1;\n    }\n    \n    // Allocate buffers with overflow checks\n    buffer = (unsigned char *)malloc(BUFFER_SIZE);\n    if (buffer == NULL) {\n        fprintf(stderr, "Memory allocation failed\\n");\n        goto cleanup;\n    }\n    \n    // Calculate destination path length with overflow check\n    size_t dir_len = strlen(UPLOAD_DIR);\n    size_t filename_len = strnlen(filename, MAX_FILENAME_LEN);\n    \n    // Check: dir_len + 1 (slash) + filename_len + 1 (null) <= MAX_PATH_LEN\n    if (dir_len > MAX_PATH_LEN - 2 || \n        filename_len > MAX_PATH_LEN - dir_len - 2) {\n        fprintf(stderr, "Path too long\\n");\n        goto cleanup;\n    }\n    \n    dest_path = (char *)malloc(MAX_PATH_LEN);\n    temp_path = (char *)malloc(MAX_PATH_LEN);\n    if (dest_path == NULL || temp_path == NULL) {\n        fprintf(stderr, "Memory allocation failed\\n");\n        goto cleanup;\n    }\n    \n    // Build paths safely using snprintf\n    int ret = snprintf(dest_path, MAX_PATH_LEN, "%s/%s", UPLOAD_DIR, filename);\n    if (ret < 0 || ret >= MAX_PATH_LEN) {\n        fprintf(stderr, "Path construction failed\\n");\n        goto cleanup;\n    }\n    \n    ret = snprintf(temp_path, MAX_PATH_LEN, "%s/.tmp_%s", UPLOAD_DIR, filename);\n    if (ret < 0 || ret >= MAX_PATH_LEN) {\n        fprintf(stderr, "Temp path construction failed\\n");\n        goto cleanup;\n    }\n    \n    // Open source file and validate handle before use\n    src = fopen(source_path, "rb");\n    if (src == NULL) {\n        fprintf(stderr, "Cannot open source file\\n");\n        goto cleanup;\n    }\n    \n    // Validate source file using fstat on the opened handle\n    if (fstat(fileno(src), &src_stat) != 0) {\n        fprintf(stderr, "Cannot stat source file\\n");\n        goto cleanup;\n    }\n    \n    // Ensure it's a regular file
    if (!S_ISREG(src_stat.st_mode)) {
        fprintf(stderr, "Source is not a regular file\\n");
        goto cleanup;
    }
    
    // Check file size limit
    if (src_stat.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "File exceeds size limit\\n");
        goto cleanup;
    }
    
    // Open destination with exclusive creation (prevents TOCTOU)
    // "wx" mode: creates exclusively, fails if exists, prevents symlink following
    dest = fopen(temp_path, "wx");
    if (dest == NULL) {
        fprintf(stderr, "Cannot create destination file\\n");
        goto cleanup;
    }
    
    // Validate destination file descriptor
    if (fstat(fileno(dest), &dest_stat) != 0) {
        fprintf(stderr, "Cannot stat destination file\\n");
        goto cleanup;
    }
    
    // Ensure destination is a regular file (not symlink or special file)
    if (!S_ISREG(dest_stat.st_mode)) {
        fprintf(stderr, "Destination is not a regular file\\n");
        goto cleanup;
    }
    
    // Copy file contents with bounds checking
    size_t bytes_read;
    size_t total_written = 0;
    
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, src)) > 0) {
        // Check for integer overflow before accumulation
        if (total_written > SIZE_MAX - bytes_read) {
            fprintf(stderr, "Size overflow detected\\n");
            goto cleanup;
        }
        
        size_t bytes_written = fwrite(buffer, 1, bytes_read, dest);
        if (bytes_written != bytes_read) {
            fprintf(stderr, "Write error\\n");
            goto cleanup;
        }
        
        total_written += bytes_written;
        
        // Recheck size limit during copy
        if (total_written > MAX_FILE_SIZE) {
            fprintf(stderr, "File size exceeded during copy\\n");
            goto cleanup;
        }
    }
    
    // Check for read errors
    if (ferror(src)) {
        fprintf(stderr, "Read error\\n");
        goto cleanup;
    }
    
    // Flush and sync to ensure data is written
    if (fflush(dest) != 0) {
        fprintf(stderr, "Flush failed\\n");
        goto cleanup;
    }
    
    if (fsync(fileno(dest)) != 0) {
        fprintf(stderr, "Fsync failed\\n");
        goto cleanup;
    }
    
    // Close destination before rename
    if (fclose(dest) != 0) {
        fprintf(stderr, "Close failed\\n");
        dest = NULL; // Prevent double close
        goto cleanup;
    }
    dest = NULL;
    
    // Atomic rename from temp to final destination
    if (rename(temp_path, dest_path) != 0) {
        fprintf(stderr, "Rename failed\\n");
        goto cleanup;
    }
    
    result = 0; // Success
    
cleanup:
    // Secure cleanup: close files and clear sensitive data
    if (src != NULL) {
        fclose(src);
    }
    if (dest != NULL) {
        fclose(dest);
        unlink(temp_path); // Remove temp file on error
    }
    if (buffer != NULL) {
        secure_zero(buffer, BUFFER_SIZE);
        free(buffer);
    }
    if (dest_path != NULL) {
        secure_zero(dest_path, MAX_PATH_LEN);
        free(dest_path);
    }
    if (temp_path != NULL) {
        secure_zero(temp_path, MAX_PATH_LEN);
        free(temp_path);
    }
    
    return result;
}

int main(void) {
    // Test case 1: Valid filename
    printf("Test 1: Valid filename\\n");
    FILE *f1 = fopen("test1.txt", "w");
    if (f1) {
        fprintf(f1, "Test content 1\\n");
        fclose(f1);
        if (upload_file("test1.txt", "uploaded1.txt") == 0) {
            printf("Success\\n");
        } else {
            printf("Failed\\n");
        }
        unlink("test1.txt");
    }
    
    // Test case 2: Invalid filename with path traversal
    printf("\\nTest 2: Path traversal attempt\\n");
    if (upload_file("test1.txt", "../evil.txt") == 0) {
        printf("Failed - should have rejected\\n");
    } else {
        printf("Success - rejected path traversal\\n");
    }
    
    // Test case 3: Invalid filename with absolute path
    printf("\\nTest 3: Absolute path attempt\\n");
    if (upload_file("test1.txt", "/etc/passwd") == 0) {
        printf("Failed - should have rejected\\n");
    } else {
        printf("Success - rejected absolute path\\n");
    }
    
    // Test case 4: Hidden file attempt
    printf("\\nTest 4: Hidden file attempt\\n");
    if (upload_file("test1.txt", ".hidden") == 0) {
        printf("Failed - should have rejected\\n");
    } else {
        printf("Success - rejected hidden file\\n");
    }
    
    // Test case 5: Valid filename with extension
    printf("\\nTest 5: Valid filename with extension\\n");
    FILE *f5 = fopen("test5.dat", "w");
    if (f5) {
        fprintf(f5, "Test content 5\\n");
        fclose(f5);
        if (upload_file("test5.dat", "data_file.dat") == 0) {
            printf("Success\\n");
        } else {
            printf("Failed\\n");
        }
        unlink("test5.dat");
    }
    
    return 0;
}
