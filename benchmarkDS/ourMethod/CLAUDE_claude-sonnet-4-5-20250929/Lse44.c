
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define SAFE_DIR "/safe/"
#define MAX_FILE_SIZE (10 * 1024 * 1024)

/**
 * Reads a file from the /safe/ directory with path traversal protection.
 * @param filename The requested filename (relative path within /safe/)
 * @param out_buffer Pointer to store allocated buffer (caller must free)
 * @param out_size Pointer to store the size of data read
 * @return 0 on success, -1 on error
 */
int read_safe_file(const char* filename, unsigned char** out_buffer, size_t* out_size) {
    int fd = -1;
    unsigned char* buffer = NULL;
    char full_path[PATH_MAX];
    char resolved_path[PATH_MAX];
    char resolved_base[PATH_MAX];
    struct stat st;
    ssize_t bytes_read;
    
    if (out_buffer) *out_buffer = NULL;
    if (out_size) *out_size = 0;
    
    if (!filename || !out_buffer || !out_size) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return -1;
    }
    
    if (strlen(filename) == 0 || strlen(filename) >= PATH_MAX - strlen(SAFE_DIR)) {
        fprintf(stderr, "Error: Invalid filename length\\n");
        return -1;
    }
    
    if (snprintf(full_path, sizeof(full_path), "%s%s", SAFE_DIR, filename) >= (int)sizeof(full_path)) {
        fprintf(stderr, "Error: Path too long\\n");
        return -1;
    }
    
    if (!realpath(SAFE_DIR, resolved_base)) {
        fprintf(stderr, "Error: Cannot resolve base directory\\n");
        return -1;
    }
    
    if (!realpath(full_path, resolved_path)) {
        fprintf(stderr, "Error: Cannot resolve requested path\\n");
        return -1;
    }
    
    if (strncmp(resolved_path, resolved_base, strlen(resolved_base)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected\\n");
        return -1;
    }
    
    fd = open(resolved_path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        fprintf(stderr, "Error: Unable to open file\\n");
        return -1;
    }
    
    if (fstat(fd, &st) != 0) {
        fprintf(stderr, "Error: Unable to stat file\\n");
        close(fd);
        return -1;
    }
    
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        close(fd);
        return -1;
    }
    
    if (st.st_size > MAX_FILE_SIZE || st.st_size < 0) {
        fprintf(stderr, "Error: File size exceeds limit or invalid\\n");
        close(fd);
        return -1;
    }
    
    buffer = (unsigned char*)malloc(st.st_size);
    if (!buffer) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        close(fd);
        return -1;
    }
    
    bytes_read = read(fd, buffer, st.st_size);
    close(fd);
    
    if (bytes_read != st.st_size) {
        fprintf(stderr, "Error: Failed to read complete file\\n");
        free(buffer);
        return -1;
    }
    
    *out_buffer = buffer;
    *out_size = (size_t)bytes_read;
    return 0;
}

int main() {
    unsigned char* buffer = NULL;
    size_t size = 0;
    
    printf("Test 1 - Valid file:\\n");
    if (read_safe_file("test.txt", &buffer, &size) == 0) {
        printf("Success: Read %zu bytes\\n", size);
        free(buffer);
        buffer = NULL;
    } else {
        printf("Failed\\n");
    }
    
    printf("\\nTest 2 - Path traversal attempt:\\n");
    if (read_safe_file("../etc/passwd", &buffer, &size) == 0) {
        printf("SECURITY BREACH\\n");
        free(buffer);
        buffer = NULL;
    } else {
        printf("Blocked as expected\\n");
    }
    
    printf("\\nTest 3 - Absolute path attempt:\\n");
    if (read_safe_file("/etc/passwd", &buffer, &size) == 0) {
        printf("SECURITY BREACH\\n");
        free(buffer);
        buffer = NULL;
    } else {
        printf("Blocked as expected\\n");
    }
    
    printf("\\nTest 4 - NULL filename:\\n");
    if (read_safe_file(NULL, &buffer, &size) == 0) {
        printf("Unexpected success\\n");
        free(buffer);
        buffer = NULL;
    } else {
        printf("Handled correctly\\n");
    }
    
    printf("\\nTest 5 - Empty filename:\\n");
    if (read_safe_file("", &buffer, &size) == 0) {
        printf("Unexpected success\\n");
        free(buffer);
        buffer = NULL;
    } else {
        printf("Handled correctly\\n");
    }
    
    return 0;
}
