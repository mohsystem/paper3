
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

#define BASE_DIR "images"
#define MAX_FILE_SIZE (10 * 1024 * 1024)  // 10MB limit

/**
 * Validates and safely resolves a file path within the base directory.
 * Returns 1 on success and fills resolvedPath, 0 on failure.
 */
int validateAndResolvePath(const char* filename, char* resolvedPath, size_t resolvedPathSize) {
    if (filename == NULL || resolvedPath == NULL || resolvedPathSize == 0) {
        return 0;
    }
    
    // Reject empty filename
    if (strlen(filename) == 0) {
        return 0;
    }
    
    // Reject path traversal patterns and directory separators
    if (strstr(filename, "..") != NULL ||
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\\\') != NULL) {\n        return 0;\n    }\n    \n    // Construct the full path\n    char fullPath[PATH_MAX];\n    int ret = snprintf(fullPath, sizeof(fullPath), "%s/%s", BASE_DIR, filename);\n    if (ret < 0 || ret >= (int)sizeof(fullPath)) {\n        return 0;\n    }\n    \n    // Resolve to canonical path\n    char* result = realpath(fullPath, resolvedPath);\n    if (result == NULL) {\n        return 0;\n    }\n    \n    // Resolve base directory\n    char resolvedBase[PATH_MAX];\n    if (realpath(BASE_DIR, resolvedBase) == NULL) {\n        return 0;\n    }\n    \n    // Ensure the resolved path starts with the base directory\n    size_t baseLen = strlen(resolvedBase);\n    if (strncmp(resolvedPath, resolvedBase, baseLen) != 0) {\n        return 0;\n    }\n    \n    return 1;\n}\n\n/**\n * Handles image file requests with secure path validation.\n * Returns pointer to allocated buffer on success, NULL on failure.\n * Caller must free the returned buffer.\n */\nunsigned char* serveImage(const char* filename, size_t* outSize) {\n    if (filename == NULL || outSize == NULL) {\n        return NULL;\n    }\n    \n    *outSize = 0;\n    \n    char validPath[PATH_MAX];\n    if (!validateAndResolvePath(filename, validPath, sizeof(validPath))) {\n        return NULL;\n    }\n    \n    // Open file with O_RDONLY and O_NOFOLLOW to prevent symlink following\n    int fd = open(validPath, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);\n    if (fd < 0) {\n        return NULL;\n    }\n    \n    // Validate the opened file descriptor\n    struct stat st;\n    if (fstat(fd, &st) != 0) {\n        close(fd);\n        return NULL;\n    }\n    \n    // Check if it's a regular file (not directory or symlink)
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return NULL;
    }
    
    // Check file size
    if (st.st_size < 0 || (size_t)st.st_size > MAX_FILE_SIZE) {
        close(fd);
        return NULL;
    }
    
    // Allocate buffer
    unsigned char* buffer = (unsigned char*)malloc(st.st_size);
    if (buffer == NULL) {
        close(fd);
        return NULL;
    }
    
    // Read file contents
    ssize_t bytesRead = read(fd, buffer, st.st_size);
    close(fd);
    
    if (bytesRead != st.st_size) {
        free(buffer);
        return NULL;
    }
    
    *outSize = st.st_size;
    return buffer;
}

int main(void) {
    // Create images directory if it doesn't exist
    mkdir(BASE_DIR, 0755);
    
    // Test case setup - create test files
    {
        FILE* f1 = fopen(BASE_DIR "/test1.jpg", "wb");
        if (f1) {
            fwrite("test image 1", 1, 12, f1);
            fclose(f1);
        }
        
        FILE* f2 = fopen(BASE_DIR "/test2.png", "wb");
        if (f2) {
            fwrite("test image 2", 1, 12, f2);
            fclose(f2);
        }
        
        FILE* f3 = fopen(BASE_DIR "/valid.gif", "wb");
        if (f3) {
            fwrite("test image 3", 1, 12, f3);
            fclose(f3);
        }
    }
    
    // Test cases
    size_t size = 0;
    unsigned char* result = NULL;
    
    printf("Test 1 - Valid file:\\n");
    result = serveImage("test1.jpg", &size);
    printf("%s\\n", result != NULL ? "SUCCESS - File served" : "FAILED");
    free(result);
    
    printf("\\nTest 2 - Another valid file:\\n");
    result = serveImage("test2.png", &size);
    printf("%s\\n", result != NULL ? "SUCCESS - File served" : "FAILED");
    free(result);
    
    printf("\\nTest 3 - Non-existent file:\\n");
    result = serveImage("nonexistent.jpg", &size);
    printf("%s\\n", result == NULL ? "SUCCESS - Rejected" : "FAILED");
    free(result);
    
    printf("\\nTest 4 - Path traversal attempt:\\n");
    result = serveImage("../etc/passwd", &size);
    printf("%s\\n", result == NULL ? "SUCCESS - Rejected" : "FAILED");
    free(result);
    
    printf("\\nTest 5 - Path with slash:\\n");
    result = serveImage("subdir/file.jpg", &size);
    printf("%s\\n", result == NULL ? "SUCCESS - Rejected" : "FAILED");
    free(result);
    
    return 0;
}
