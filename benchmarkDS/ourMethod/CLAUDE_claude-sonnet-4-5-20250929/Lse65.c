
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>

// Rules#2: Input validation - validate file extension and size
// Rules#7: Files and TOCTOU - open first, validate, operate on handle
// Rules#8: Memory safety - check allocations, bound operations

#define MAX_FILENAME_LEN 255
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10MB limit
#define UPLOAD_DIR "uploads"
#define BUFFER_SIZE 8192

// Rules#8: Initialize pointers to NULL
static void secure_zero(void *ptr, size_t len) {
    if (ptr != NULL && len > 0) {
        volatile unsigned char *p = ptr;
        while (len--) *p++ = 0;
    }
}

// Rules#2: Validate filename - allow list approach
static int validate_filename(const char *filename) {
    if (filename == NULL || strlen(filename) == 0) return 0;
    
    size_t len = strlen(filename);
    // Rules#2: Check length
    if (len > MAX_FILENAME_LEN || len < 5) return 0;
    
    // Rules#2: Check for path traversal
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL || 
        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    // Rules#2: Validate PDF extension\n    const char *ext = strrchr(filename, '.');\n    if (ext == NULL || strcmp(ext, ".pdf") != 0) return 0;\n    \n    // Rules#2: Allow only alphanumeric, underscore, dash, dot\n    for (size_t i = 0; i < len; i++) {\n        char c = filename[i];\n        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || \n              (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.')) {\n            return 0;\n        }\n    }\n    \n    return 1;\n}\n\n// Rules#7: Safe file operations with TOCTOU prevention\nstatic int save_uploaded_file(const char *filename, const unsigned char *data, size_t data_len) {\n    int dirfd = -1;\n    int fd = -1;\n    int ret = -1;\n    char safe_path[PATH_MAX];\n    FILE *fp = NULL;\n    \n    // Rules#2: Validate inputs\n    if (filename == NULL || data == NULL || data_len == 0) {\n        fprintf(stderr, "Invalid input parameters\\n");\n        return -1;\n    }\n    \n    // Rules#2: Validate filename\n    if (!validate_filename(filename)) {\n        fprintf(stderr, "Invalid filename\\n");\n        return -1;\n    }\n    \n    // Rules#2: Check file size limit\n    if (data_len > MAX_FILE_SIZE) {\n        fprintf(stderr, "File too large\\n");\n        return -1;\n    }\n    \n    // Rules#7: Create upload directory with restrictive permissions\n    if (mkdir(UPLOAD_DIR, 0700) != 0 && errno != EEXIST) {\n        fprintf(stderr, "Failed to create upload directory\\n");\n        return -1;\n    }\n    \n    // Rules#7: Open directory for use with openat\n    dirfd = open(UPLOAD_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);\n    if (dirfd < 0) {\n        fprintf(stderr, "Failed to open upload directory\\n");\n        return -1;\n    }\n    \n    // Rules#7: Build safe path\n    if (snprintf(safe_path, sizeof(safe_path), "%s", filename) >= (int)sizeof(safe_path)) {\n        fprintf(stderr, "Path too long\\n");\n        goto cleanup;\n    }\n    \n    // Rules#7: Open file atomically with O_EXCL to prevent overwrites\n    // O_NOFOLLOW prevents symlink following\n    fd = openat(dirfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);\n    if (fd < 0) {\n        fprintf(stderr, "Failed to create file\\n");\n        goto cleanup;\n    }\n    \n    // Rules#7: Validate opened file with fstat\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        fprintf(stderr, "Failed to stat file\\n");\n        goto cleanup;\n    }\n    \n    // Rules#7: Ensure it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Not a regular file\\n");
        goto cleanup;
    }
    
    // Rules#8: Write data with bounds checking
    size_t written = 0;
    while (written < data_len) {
        // Rules#8: Check for integer overflow
        size_t to_write = data_len - written;
        if (to_write > BUFFER_SIZE) to_write = BUFFER_SIZE;
        
        ssize_t n = write(fd, data + written, to_write);
        if (n <= 0) {
            fprintf(stderr, "Write failed\\n");
            goto cleanup;
        }
        written += n;
    }
    
    // Rules#7: Ensure data is written to disk
    if (fsync(fd) != 0) {
        fprintf(stderr, "Failed to sync file\\n");
        goto cleanup;
    }
    
    ret = 0;
    
cleanup:
    // Rules#8: Cleanup in reverse order
    if (fd >= 0) close(fd);
    if (dirfd >= 0) close(dirfd);
    
    // Rules#7: If operation failed, remove partial file
    if (ret != 0 && fd >= 0) {
        unlinkat(dirfd >= 0 ? dirfd : AT_FDCWD, filename, 0);
    }
    
    return ret;
}

// Test function
int main(void) {
    printf("PDF Upload Handler - Test Cases\\n");
    printf("================================\\n\\n");
    
    // Test Case 1: Valid PDF upload
    printf("Test 1: Valid PDF file\\n");
    const unsigned char test_data1[] = "%PDF-1.4 test content";
    if (save_uploaded_file("test1.pdf", test_data1, sizeof(test_data1) - 1) == 0) {
        printf("PASS: File uploaded successfully\\n\\n");
    } else {
        printf("FAIL: Upload failed\\n\\n");
    }
    
    // Test Case 2: Invalid extension
    printf("Test 2: Invalid file extension\\n");
    const unsigned char test_data2[] = "malicious content";
    if (save_uploaded_file("test2.exe", test_data2, sizeof(test_data2) - 1) != 0) {
        printf("PASS: Rejected non-PDF file\\n\\n");
    } else {
        printf("FAIL: Should reject non-PDF\\n\\n");
    }
    
    // Test Case 3: Path traversal attempt
    printf("Test 3: Path traversal attack\\n");
    const unsigned char test_data3[] = "attack data";
    if (save_uploaded_file("../../../etc/passwd.pdf", test_data3, sizeof(test_data3) - 1) != 0) {
        printf("PASS: Blocked path traversal\\n\\n");
    } else {
        printf("FAIL: Should block path traversal\\n\\n");
    }
    
    // Test Case 4: Empty filename
    printf("Test 4: Empty filename\\n");
    const unsigned char test_data4[] = "data";
    if (save_uploaded_file("", test_data4, sizeof(test_data4) - 1) != 0) {
        printf("PASS: Rejected empty filename\\n\\n");
    } else {
        printf("FAIL: Should reject empty filename\\n\\n");
    }
    
    // Test Case 5: NULL data
    printf("Test 5: NULL data pointer\\n");
    if (save_uploaded_file("test5.pdf", NULL, 100) != 0) {
        printf("PASS: Rejected NULL data\\n\\n");
    } else {
        printf("FAIL: Should reject NULL data\\n\\n");
    }
    
    return 0;
}
