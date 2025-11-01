
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define SAFE_DIRECTORY "/safe/directory"
#define MAX_FILE_SIZE (10 * 1024 * 1024) /* 10MB limit */

/**
 * Reads a file from the safe directory with proper security controls.
 * @param filename The requested filename (relative path)
 * @param buffer Pointer to store allocated buffer (caller must free)
 * @param size Pointer to store size of data read
 * @return 0 on success, -1 on failure
 */
int read_file_from_safe_directory(const char* filename, unsigned char** buffer, size_t* size) {
    int fd = -1;
    unsigned char* data = NULL;
    struct stat st;
    char safe_dir_real[PATH_MAX];
    char requested_path[PATH_MAX];
    char requested_real[PATH_MAX];
    ssize_t bytes_read = 0;
    ssize_t total_read = 0;
    
    /* Initialize output parameters to NULL */
    if (buffer != NULL) *buffer = NULL;
    if (size != NULL) *size = 0;
    
    /* Input validation: reject NULL or empty filename */
    if (filename == NULL || filename[0] == '\\0') {
        fprintf(stderr, "Error: Filename cannot be NULL or empty\\n");
        return -1;
    }
    
    /* Resolve the safe directory to absolute path */
    if (realpath(SAFE_DIRECTORY, safe_dir_real) == NULL) {
        fprintf(stderr, "Error: Cannot resolve safe directory\\n");
        return -1;
    }
    
    /* Construct the requested path */
    if (snprintf(requested_path, sizeof(requested_path), "%s/%s", 
                 safe_dir_real, filename) >= (int)sizeof(requested_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        return -1;
    }
    
    /* Resolve to absolute path to check for path traversal */
    if (realpath(requested_path, requested_real) == NULL) {
        fprintf(stderr, "Error: File does not exist or cannot be resolved\\n");
        return -1;
    }
    
    /* Security check: ensure resolved path is within safe directory */
    size_t safe_dir_len = strlen(safe_dir_real);
    if (strncmp(requested_real, safe_dir_real, safe_dir_len) != 0 ||
        (requested_real[safe_dir_len] != '/' && requested_real[safe_dir_len] != '\\0')) {
        fprintf(stderr, "Error: Access denied - path traversal detected\\n");
        return -1;
    }
    
    /* Open file with O_RDONLY, O_CLOEXEC, and O_NOFOLLOW (race-safe) */
    fd = open(requested_real, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return -1;
    }
    
    /* Validate the opened file descriptor with fstat */
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Cannot stat file\\n");
        goto cleanup_error;
    }
    
    /* Check if it's a regular file */
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        goto cleanup_error;
    }
    
    /* Check file size to prevent excessive memory allocation */
    if (st.st_size > (off_t)MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size\\n");
        goto cleanup_error;
    }
    
    /* Allocate buffer for file contents */
    data = (unsigned char*)malloc(st.st_size);
    if (data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        goto cleanup_error;
    }
    
    /* Read file contents */
    while (total_read < st.st_size) {
        bytes_read = read(fd, data + total_read, st.st_size - total_read);
        if (bytes_read < 0) {
            fprintf(stderr, "Error: Reading file failed\\n");
            goto cleanup_error;
        }
        if (bytes_read == 0) {
            break; /* EOF */
        }
        total_read += bytes_read;
    }
    
    close(fd);
    
    /* Set output parameters */
    if (buffer != NULL) *buffer = data;
    if (size != NULL) *size = (size_t)total_read;
    
    return 0;

cleanup_error:
    if (fd >= 0) close(fd);
    if (data != NULL) free(data);
    return -1;
}

int main(void) {
    unsigned char* content = NULL;
    size_t content_size = 0;
    
    /* Test case 1: Valid file in safe directory */
    printf("Test 1 - Valid file: ");
    if (read_file_from_safe_directory("test.txt", &content, &content_size) == 0) {
        printf("Success (%zu bytes)\\n", content_size);
        free(content);
        content = NULL;
    } else {
        printf("Failed\\n");
    }
    
    /* Test case 2: Attempt path traversal attack */
    printf("Test 2 - Path traversal: ");
    if (read_file_from_safe_directory("../../../etc/passwd", &content, &content_size) == 0) {
        printf("Failed (should be blocked)\\n");
        free(content);
        content = NULL;
    } else {
        printf("Blocked successfully\\n");
    }
    
    /* Test case 3: NULL filename */
    printf("Test 3 - NULL filename: ");
    if (read_file_from_safe_directory(NULL, &content, &content_size) == 0) {
        printf("Failed (should be rejected)\\n");
        free(content);
        content = NULL;
    } else {
        printf("Rejected successfully\\n");
    }
    
    /* Test case 4: Empty filename */
    printf("Test 4 - Empty filename: ");
    if (read_file_from_safe_directory("", &content, &content_size) == 0) {
        printf("Failed (should be rejected)\\n");
        free(content);
        content = NULL;
    } else {
        printf("Rejected successfully\\n");
    }
    
    /* Test case 5: Subdirectory access (valid) */
    printf("Test 5 - Subdirectory file: ");
    if (read_file_from_safe_directory("subdir/file.txt", &content, &content_size) == 0) {
        printf("Success (%zu bytes)\\n", content_size);
        free(content);
        content = NULL;
    } else {
        printf("Failed\\n");
    }
    
    return 0;
}
