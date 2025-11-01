
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

/* Security: Validate and sanitize filename to prevent path traversal */
int validate_filename(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }
    
    /* Security: Reject path traversal patterns */
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL || 
        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/* Security: Validate path is within base directory */\nint validate_path_within_base(const char* base_dir, const char* filename, char* resolved_path, size_t path_size) {\n    if (base_dir == NULL || filename == NULL || resolved_path == NULL) {\n        return 0;\n    }\n    \n    /* Security: Build and normalize the full path */\n    char temp_path[PATH_MAX];\n    int result = snprintf(temp_path, sizeof(temp_path), "%s/%s", base_dir, filename);\n    if (result < 0 || result >= (int)sizeof(temp_path)) {\n        return 0;\n    }\n    \n    /* Security: Resolve to canonical path */\n    char* real = realpath(temp_path, resolved_path);\n    if (real == NULL) {\n        /* If file doesn't exist, resolve parent and append filename */
        char parent[PATH_MAX];
        result = snprintf(parent, sizeof(parent), "%s", base_dir);
        if (result < 0 || result >= (int)sizeof(parent)) {
            return 0;
        }
        
        real = realpath(parent, resolved_path);
        if (real == NULL) {
            return 0;
        }
        
        size_t len = strlen(resolved_path);
        result = snprintf(resolved_path + len, path_size - len, "/%s", filename);
        if (result < 0 || result >= (int)(path_size - len)) {
            return 0;
        }
    }
    
    /* Security: Verify resolved path starts with base directory */
    char base_real[PATH_MAX];
    if (realpath(base_dir, base_real) == NULL) {
        return 0;
    }
    
    if (strncmp(resolved_path, base_real, strlen(base_real)) != 0) {
        return 0;
    }
    
    return 1;
}

/* Secure file write with proper permissions and TOCTOU prevention */
int write_to_config_file(const char* base_dir, const char* filename, const char* content) {
    /* Security: Validate all inputs */
    if (base_dir == NULL || filename == NULL || content == NULL) {
        fprintf(stderr, "Error: Invalid input parameters\\n");
        return -1;
    }
    
    /* Security: Validate filename format */
    if (!validate_filename(filename)) {
        fprintf(stderr, "Error: Invalid filename format\\n");
        return -1;
    }
    
    /* Security: Validate path is within base directory */
    char resolved_path[PATH_MAX];
    if (!validate_path_within_base(base_dir, filename, resolved_path, sizeof(resolved_path))) {
        fprintf(stderr, "Error: Path validation failed\\n");
        return -1;
    }
    
    /* Security: Open file with O_CREAT and restrictive permissions */
    /* S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH = 0644 (owner rw, group r, others r) */
    /* O_NOFOLLOW prevents following symlinks (TOCTOU protection) */
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int flags = O_CREAT | O_WRONLY | O_TRUNC | O_CLOEXEC;
    
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
    
    /* Security: Open with restrictive permissions atomically */
    int fd = open(resolved_path, flags, mode);
    if (fd < 0) {
        fprintf(stderr, "Error: Failed to open file: %s\\n", strerror(errno));
        return -1;
    }
    
    /* Security: Validate the opened file descriptor (already opened, no TOCTOU) */
    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(stderr, "Error: fstat failed\\n");
        close(fd);
        return -1;
    }
    
    /* Security: Ensure it's a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return -1;
    }
    
    /* Security: Write content using the already-opened fd (no TOCTOU) */
    size_t content_len = strlen(content);
    ssize_t bytes_written = 0;
    ssize_t total_written = 0;
    
    while (total_written < (ssize_t)content_len) {
        bytes_written = write(fd, content + total_written, content_len - total_written);
        if (bytes_written < 0) {
            fprintf(stderr, "Error: Write failed: %s\\n", strerror(errno));
            close(fd);
            return -1;
        }
        total_written += bytes_written;
    }
    
    /* Security: Force write to disk */
    if (fsync(fd) < 0) {
        fprintf(stderr, "Warning: fsync failed: %s\\n", strerror(errno));
    }
    
    /* Security: Close the file descriptor */
    if (close(fd) < 0) {
        fprintf(stderr, "Warning: close failed: %s\\n", strerror(errno));
        return -1;
    }
    
    return 0;
}

int main(void) {
    const char* base_dir = "/tmp";
    
    /* Test case 1: Basic write */
    printf("Test 1: ");
    if (write_to_config_file(base_dir, "important_config", "important_config") == 0) {
        printf("Passed - Basic write successful\\n");
    } else {
        printf("Failed - Basic write failed\\n");
    }
    
    /* Test case 2: Write with different content */
    printf("Test 2: ");
    if (write_to_config_file(base_dir, "test_config2", "test_data_123") == 0) {
        printf("Passed - Different content write successful\\n");
    } else {
        printf("Failed - Different content write failed\\n");
    }
    
    /* Test case 3: Empty content */
    printf("Test 3: ");
    if (write_to_config_file(base_dir, "empty_config", "") == 0) {
        printf("Passed - Empty content write successful\\n");
    } else {
        printf("Failed - Empty content write failed\\n");
    }
    
    /* Test case 4: Path traversal attempt (should fail) */
    printf("Test 4: ");
    if (write_to_config_file(base_dir, "../etc/passwd", "malicious") != 0) {
        printf("Passed - Path traversal blocked\\n");
    } else {
        printf("Failed - Path traversal not blocked\\n");
    }
    
    /* Test case 5: NULL filename (should fail) */
    printf("Test 5: ");
    if (write_to_config_file(base_dir, NULL, "content") != 0) {
        printf("Passed - NULL filename rejected\\n");
    } else {
        printf("Failed - NULL filename not rejected\\n");
    }
    
    return 0;
}
