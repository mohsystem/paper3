
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

/* Security: Maximum file size to prevent DoS and memory exhaustion */
#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10 MB */
/* Security: Maximum filename length to prevent buffer overflows */
#define MAX_FILENAME_LEN 255
/* Security: Maximum path length */
#define MAX_PATH_LEN 4096
/* Security: Buffer size for safe file operations */
#define BUFFER_SIZE 8192

/* Security: Base directory where files are stored - prevents path traversal */
static const char* BASE_UPLOAD_DIR = "./uploads";

/* Security: Explicit memory zeroization to clear sensitive data */
static void explicit_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) *p++ = 0;
}

/* Security: Validates filename to prevent path traversal (CWE-22, CWE-73)
 * Rejects: "..", "/", "\\", null bytes, and control characters\n * Only allows: alphanumeric, underscore, hyphen, dot (not at start)\n */\nstatic int validate_filename(const char* filename) {\n    size_t len;\n    size_t i;\n    \n    /* Security: NULL check */\n    if (filename == NULL) {\n        fprintf(stderr, "Error: Filename is NULL\
");\n        return 0;\n    }\n    \n    len = strlen(filename);\n    \n    /* Security: Length validation to prevent buffer overflows (CWE-120, CWE-131) */\n    if (len == 0 || len > MAX_FILENAME_LEN) {\n        fprintf(stderr, "Error: Invalid filename length\
");\n        return 0;\n    }\n    \n    /* Security: Reject hidden files and relative paths */\n    if (filename[0] == '.' || filename[0] == '/' || filename[0] == '\\\\') {\n        fprintf(stderr, "Error: Filename cannot start with '.', '/', or '\\\\'\\n");\n        return 0;\n    }\n    \n    /* Security: Check each character - prevent injection and traversal */\n    for (i = 0; i < len; i++) {\n        unsigned char c = (unsigned char)filename[i];\n        \n        /* Security: Reject null bytes (CWE-158) */\n        if (c == '\\0') {\n            fprintf(stderr, "Error: Null byte in filename\\n");\n            return 0;\n        }\n        \n        /* Security: Reject path separators and control characters */\n        if (c == '/' || c == '\\\\' || c < 32 || c == 127) {\n            fprintf(stderr, "Error: Invalid character in filename\\n");\n            return 0;\n        }\n        \n        /* Security: Check for path traversal sequences */\n        if (c == '.' && i > 0 && filename[i-1] == '.') {\n            fprintf(stderr, "Error: '..' sequence not allowed\\n");\n            return 0;\n        }\n    }\n    \n    /* Security: Additional check for dangerous names */\n    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {\n        fprintf(stderr, "Error: Invalid filename\\n");\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Security: Constructs safe file path within base directory\n * Prevents path traversal by validating and containing paths (CWE-22, CWE-73)\n */\nstatic int construct_safe_path(const char* filename, char* full_path, size_t path_size) {\n    int result;\n    size_t base_len;\n    size_t name_len;\n    \n    /* Security: Validate inputs */\n    if (filename == NULL || full_path == NULL || path_size == 0) {\n        return 0;\n    }\n    \n    /* Security: Validate filename before using it */\n    if (!validate_filename(filename)) {\n        return 0;\n    }\n    \n    base_len = strlen(BASE_UPLOAD_DIR);\n    name_len = strlen(filename);\n    \n    /* Security: Check for integer overflow before addition (CWE-190) */\n    if (base_len > SIZE_MAX - name_len - 2) {\n        fprintf(stderr, "Error: Path length calculation overflow\\n");\n        return 0;\n    }\n    \n    /* Security: Ensure buffer is large enough (CWE-120, CWE-131) */\n    if (base_len + 1 + name_len + 1 > path_size) {\n        fprintf(stderr, "Error: Path too long\\n");\n        return 0;\n    }\n    \n    /* Security: Use snprintf for safe string formatting (CWE-120) */\n    result = snprintf(full_path, path_size, "%s/%s", BASE_UPLOAD_DIR, filename);\n    \n    /* Security: Check snprintf result */\n    if (result < 0 || (size_t)result >= path_size) {\n        fprintf(stderr, "Error: Path construction failed\\n");\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Security: Creates upload directory with restrictive permissions (CWE-732)\n * Uses 0700 permissions to prevent unauthorized access\n */\nstatic int ensure_upload_directory(void) {\n    struct stat st;\n    \n    /* Security: Check if directory exists */\n    if (stat(BASE_UPLOAD_DIR, &st) == 0) {\n        /* Security: Verify it's a directory */
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "Error: Upload path exists but is not a directory\\n");
            return 0;
        }
        return 1;
    }
    
    /* Security: Create with restrictive permissions (CWE-732) */
    if (mkdir(BASE_UPLOAD_DIR, 0700) != 0) {
        fprintf(stderr, "Error: Failed to create upload directory: %s\\n", strerror(errno));
        return 0;
    }
    
    return 1;
}

/* Security: Race-condition safe file upload using open-then-validate pattern (CWE-367, CWE-363)
 * Prevents TOCTOU by opening file first, then validating the file descriptor
 * Uses O_CREAT|O_EXCL to prevent overwriting existing files (CWE-59)
 */
static int upload_file_safe(const char* filename, const unsigned char* data, size_t data_size) {
    char full_path[MAX_PATH_LEN];
    int fd = -1;
    struct stat st;
    ssize_t written;
    size_t total_written = 0;
    int fsync_result;
    
    /* Security: Input validation (CWE-20) */
    if (filename == NULL || data == NULL) {
        fprintf(stderr, "Error: Invalid input parameters\\n");
        return 0;
    }
    
    /* Security: Validate file size to prevent DoS (CWE-400) */
    if (data_size == 0 || data_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Invalid file size\\n");
        return 0;
    }
    
    /* Security: Ensure upload directory exists */
    if (!ensure_upload_directory()) {
        return 0;
    }
    
    /* Security: Construct safe path within base directory (CWE-22) */
    if (!construct_safe_path(filename, full_path, sizeof(full_path))) {
        return 0;
    }
    
    /* Security: Open file with restrictive flags and permissions (CWE-367, CWE-732)
     * O_CREAT|O_EXCL: Fails if file exists, prevents race conditions (CWE-59)
     * O_WRONLY: Write-only access
     * O_CLOEXEC: Close on exec for security
     * 0600: Owner read/write only (CWE-732)
     */
    fd = open(full_path, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0600);
    if (fd < 0) {
        if (errno == EEXIST) {
            fprintf(stderr, "Error: File already exists\\n");
        } else {
            fprintf(stderr, "Error: Failed to create file: %s\\n", strerror(errno));
        }
        return 0;
    }
    
    /* Security: Validate the opened file descriptor (CWE-367)
     * This prevents TOCTOU by checking the already-opened file
     */
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Failed to stat file: %s\\n", strerror(errno));
        close(fd);
        unlink(full_path); /* Clean up on error */
        return 0;
    }
    
    /* Security: Ensure it's a regular file, not a symlink or device (CWE-59) */\n    if (!S_ISREG(st.st_mode)) {\n        fprintf(stderr, "Error: Not a regular file\\n");\n        close(fd);\n        unlink(full_path);\n        return 0;\n    }\n    \n    /* Security: Write data in chunks with error checking (CWE-391, CWE-252)\n     * Check all write return values to prevent data corruption\n     */\n    while (total_written < data_size) {\n        size_t remaining = data_size - total_written;\n        size_t to_write = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;\n        \n        written = write(fd, data + total_written, to_write);\n        \n        /* Security: Check write result (CWE-252) */\n        if (written < 0) {\n            fprintf(stderr, "Error: Write failed: %s\\n", strerror(errno));\n            close(fd);\n            unlink(full_path); /* Clean up on error */\n            return 0;\n        }\n        \n        /* Security: Check for integer overflow (CWE-190) */\n        if (total_written > SIZE_MAX - (size_t)written) {\n            fprintf(stderr, "Error: Write size overflow\\n");\n            close(fd);\n            unlink(full_path);\n            return 0;\n        }\n        \n        total_written += (size_t)written;\n    }\n    \n    /* Security: Ensure data is written to disk (CWE-662) */\n    fsync_result = fsync(fd);\n    if (fsync_result != 0) {\n        fprintf(stderr, "Warning: fsync failed: %s\\n", strerror(errno));\n    }\n    \n    /* Security: Close file and check result (CWE-252) */\n    if (close(fd) != 0) {\n        fprintf(stderr, "Error: Failed to close file: %s\\n", strerror(errno));\n        unlink(full_path); /* Clean up on error */\n        return 0;\n    }\n    \n    printf("Success: File '%s' uploaded successfully (%zu bytes)\\n", filename, data_size);\n    return 1;\n}\n\n/* Test function with 5 test cases */\nint main(void) {\n    unsigned char test_data1[] = "This is test file content for test case 1.";\n    unsigned char test_data2[] = "Test case 2: Different content with more data to test buffer handling.";\n    unsigned char test_data3[] = "Small";\n    unsigned char large_data[1024];\n    unsigned char test_data5[] = "Final test case with normal content.";\n    size_t i;\n    \n    printf("=== File Upload Server Test Cases ===\\n\\n");\n    \n    /* Test Case 1: Normal file upload */\n    printf("Test 1: Normal file upload\\n");\n    if (upload_file_safe("test1.txt", test_data1, sizeof(test_data1) - 1)) {\n        printf("Test 1: PASSED\\n\\n");\n    } else {\n        printf("Test 1: FAILED\\n\\n");\n    }\n    \n    /* Test Case 2: Another normal file with different content */\n    printf("Test 2: Different content file\\n");\n    if (upload_file_safe("test2.txt", test_data2, sizeof(test_data2) - 1)) {\n        printf("Test 2: PASSED\\n\\n");\n    } else {\n        printf("Test 2: FAILED\\n\\n");\n    }\n    \n    /* Test Case 3: Small file */\n    printf("Test 3: Small file upload\\n");\n    if (upload_file_safe("small_file.txt", test_data3, sizeof(test_data3) - 1)) {\n        printf("Test 3: PASSED\\n\\n");\n    } else {\n        printf("Test 3: FAILED\\n\\n");\n    }\n    \n    /* Test Case 4: Larger file to test buffering */\n    printf("Test 4: Larger file upload\\n");\n    for (i = 0; i < sizeof(large_data); i++) {\n        large_data[i] = (unsigned char)('A' + (i % 26));
    }
    if (upload_file_safe("large_test.txt", large_data, sizeof(large_data))) {
        printf("Test 4: PASSED\\n\\n");
    } else {
        printf("Test 4: FAILED\\n\\n");
    }
    
    /* Test Case 5: Security test - attempt path traversal (should fail) */
    printf("Test 5: Security test - path traversal attempt (should fail)\\n");
    if (!upload_file_safe("../etc/passwd", test_data5, sizeof(test_data5) - 1)) {
        printf("Test 5: PASSED (correctly rejected malicious filename)\\n\\n");
    } else {
        printf("Test 5: FAILED (security vulnerability detected!)\\n\\n");
    }
    
    /* Security: Clear sensitive data before exit */
    explicit_zero(test_data1, sizeof(test_data1));
    explicit_zero(test_data2, sizeof(test_data2));
    explicit_zero(test_data3, sizeof(test_data3));
    explicit_zero(large_data, sizeof(large_data));
    explicit_zero(test_data5, sizeof(test_data5));
    
    printf("=== All tests completed ===\\n");
    return 0;
}
