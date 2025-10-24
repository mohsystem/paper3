
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#define MAX_PATH_LEN 4096
#define MAX_FILE_SIZE (100 * 1024 * 1024) // 100MB limit

// Structure to hold file data
typedef struct {
    unsigned char* data;
    size_t size;
} FileBuffer;

// Initialize FileBuffer to NULL
void initFileBuffer(FileBuffer* fb) {
    if (fb != NULL) {
        fb->data = NULL;
        fb->size = 0;
    }
}

// Free FileBuffer memory securely
void freeFileBuffer(FileBuffer* fb) {
    if (fb != NULL && fb->data != NULL) {
        // Clear sensitive data before freeing
        memset(fb->data, 0, fb->size);
        free(fb->data);
        fb->data = NULL;
        fb->size = 0;
    }
}

// Check if string contains dangerous sequences
int containsDangerousSequence(const char* path) {
    if (path == NULL) {
        return 1;
    }
    
    // Check for null bytes in path
    size_t len = strlen(path);
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '\\0' && i < len - 1) {
            return 1;
        }
    }
    
    // Check for path traversal sequences
    if (strstr(path, "..") != NULL) {
        return 1;
    }
    
    return 0;
}

// Normalize and validate path against base directory
// Returns 0 on success, -1 on failure
int normalizePath(const char* basePath, const char* userPath, char* outPath, size_t outSize) {
    // Validate inputs
    if (basePath == NULL || userPath == NULL || outPath == NULL || outSize == 0) {
        return -1;
    }
    
    // Check for empty paths
    if (strlen(basePath) == 0 || strlen(userPath) == 0) {
        return -1;
    }
    
    // Validate buffer size
    if (outSize < PATH_MAX) {
        return -1;
    }
    
    // Check for dangerous sequences
    if (containsDangerousSequence(userPath) || containsDangerousSequence(basePath)) {
        return -1;
    }
    
    // Reject absolute paths that don't start with base\n    if (userPath[0] == '/') {\n        if (strncmp(userPath, basePath, strlen(basePath)) != 0) {\n            return -1;\n        }\n    }\n    \n    // Construct full path with bounds checking\n    char fullPath[PATH_MAX];\n    int result = snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, userPath);\n    if (result < 0 || (size_t)result >= sizeof(fullPath)) {\n        return -1;\n    }\n    \n    // Resolve to canonical path to prevent symlink attacks\n    char* resolved = realpath(fullPath, NULL);\n    if (resolved == NULL) {\n        return -1;\n    }\n    \n    // Resolve base path\n    char* resolvedBase = realpath(basePath, NULL);\n    if (resolvedBase == NULL) {\n        free(resolved);\n        return -1;\n    }\n    \n    // Verify resolved path is within base directory\n    size_t baseLen = strlen(resolvedBase);\n    if (strncmp(resolved, resolvedBase, baseLen) != 0) {\n        free(resolved);\n        free(resolvedBase);\n        return -1;\n    }\n    \n    // Copy to output buffer with bounds check\n    if (strlen(resolved) >= outSize) {\n        free(resolved);\n        free(resolvedBase);\n        return -1;\n    }\n    \n    strncpy(outPath, resolved, outSize - 1);\n    outPath[outSize - 1] = '\\0';\n    \n    free(resolved);\n    free(resolvedBase);\n    return 0;\n}\n\n// Verify file is regular file, not symlink or directory\nint isRegularFile(const char* path) {\n    if (path == NULL) {\n        return 0;\n    }\n    \n    struct stat statbuf;\n    // Use lstat to detect symlinks\n    if (lstat(path, &statbuf) != 0) {\n        return 0;\n    }\n    \n    return S_ISREG(statbuf.st_mode);\n}\n\n// Read file securely with bounds checking\nint readFileSecurely(const char* filePath, FileBuffer* buffer) {\n    FILE* file = NULL;\n    \n    // Validate inputs\n    if (filePath == NULL || buffer == NULL) {\n        return -1;\n    }\n    \n    // Initialize buffer\n    initFileBuffer(buffer);\n    \n    // Verify it's a regular file
    if (!isRegularFile(filePath)) {
        fprintf(stderr, "Not a regular file or is a symlink\\n");
        return -1;
    }
    
    // Open file for reading in binary mode
    file = fopen(filePath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Failed to open input file: %s\\n", strerror(errno));
        return -1;
    }
    
    // Get file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek to end of file\\n");
        fclose(file);
        return -1;
    }
    
    long fileSize = ftell(file);
    if (fileSize < 0) {
        fprintf(stderr, "Failed to get file size\\n");
        fclose(file);
        return -1;
    }
    
    // Validate file size to prevent excessive memory allocation
    if (fileSize > MAX_FILE_SIZE) {
        fprintf(stderr, "File too large\\n");
        fclose(file);
        return -1;
    }
    
    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Failed to seek to beginning of file\\n");
        fclose(file);
        return -1;
    }
    
    // Allocate buffer with overflow check
    if (fileSize > 0) {
        buffer->data = (unsigned char*)malloc((size_t)fileSize);
        if (buffer->data == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            fclose(file);
            return -1;
        }
        buffer->size = (size_t)fileSize;
        
        // Read file content with bounds checking
        size_t bytesRead = fread(buffer->data, 1, buffer->size, file);
        if (bytesRead != buffer->size) {
            fprintf(stderr, "Failed to read complete file\\n");
            freeFileBuffer(buffer);
            fclose(file);
            return -1;
        }
    }
    
    fclose(file);
    return 0;
}

// Write file atomically to prevent TOCTOU vulnerabilities
int writeFileAtomically(const char* destPath, const FileBuffer* buffer) {
    int fd = -1;
    char tempPath[PATH_MAX];
    
    // Validate inputs
    if (destPath == NULL || buffer == NULL || buffer->data == NULL) {
        return -1;
    }
    
    // Create temporary file path with bounds checking
    int result = snprintf(tempPath, sizeof(tempPath), "%s.tmp.XXXXXX", destPath);
    if (result < 0 || (size_t)result >= sizeof(tempPath)) {
        fprintf(stderr, "Failed to create temporary path\\n");
        return -1;
    }
    
    // Create temporary file with secure permissions (0600)
    fd = mkstemp(tempPath);
    if (fd == -1) {
        fprintf(stderr, "Failed to create temporary file: %s\\n", strerror(errno));
        return -1;
    }
    
    // Set secure permissions explicitly
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
        fprintf(stderr, "Failed to set file permissions: %s\\n", strerror(errno));
        close(fd);
        unlink(tempPath);
        return -1;
    }
    
    // Write data to temporary file with bounds checking
    size_t totalWritten = 0;
    while (totalWritten < buffer->size) {
        ssize_t written = write(fd, buffer->data + totalWritten, buffer->size - totalWritten);
        if (written < 0) {
            if (errno == EINTR) {
                continue; // Retry on interrupt
            }
            fprintf(stderr, "Failed to write to temporary file: %s\\n", strerror(errno));
            close(fd);
            unlink(tempPath);
            return -1;
        }
        totalWritten += (size_t)written;
    }
    
    // Flush to disk to ensure data persistence
    if (fsync(fd) != 0) {
        fprintf(stderr, "Failed to sync file to disk: %s\\n", strerror(errno));
        close(fd);
        unlink(tempPath);
        return -1;
    }
    
    close(fd);
    
    // Atomically rename to final destination
    if (rename(tempPath, destPath) != 0) {
        fprintf(stderr, "Failed to rename temporary file: %s\\n", strerror(errno));
        unlink(tempPath);
        return -1;
    }
    
    return 0;
}

// Main processing function
int processFile(const char* baseDir, const char* inputFile, 
                const char* tempDir, const char* outputFile) {
    char validatedInput[PATH_MAX];
    char validatedOutput[PATH_MAX];
    FileBuffer fileBuffer;
    
    // Initialize buffer
    initFileBuffer(&fileBuffer);
    
    // Validate inputs
    if (baseDir == NULL || inputFile == NULL || tempDir == NULL || outputFile == NULL) {
        fprintf(stderr, "Error: NULL parameter provided\\n");
        return -1;
    }
    
    // Validate and normalize input path
    if (normalizePath(baseDir, inputFile, validatedInput, sizeof(validatedInput)) != 0) {
        fprintf(stderr, "Error: Invalid input path or path escapes base directory\\n");
        return -1;
    }
    
    // Validate and normalize output path
    if (normalizePath(tempDir, outputFile, validatedOutput, sizeof(validatedOutput)) != 0) {
        fprintf(stderr, "Error: Invalid output path or path escapes temp directory\\n");
        return -1;
    }
    
    // Read file securely
    if (readFileSecurely(validatedInput, &fileBuffer) != 0) {
        fprintf(stderr, "Error: Failed to read input file\\n");
        return -1;
    }
    
    // Write file atomically to prevent TOCTOU
    if (writeFileAtomically(validatedOutput, &fileBuffer) != 0) {
        fprintf(stderr, "Error: Failed to write output file\\n");
        freeFileBuffer(&fileBuffer);
        return -1;
    }
    
    printf("Successfully copied file from %s to %s\\n", validatedInput, validatedOutput);
    
    // Clean up
    freeFileBuffer(&fileBuffer);
    return 0;
}

int main(void) {
    // Test cases with proper error handling
    printf("=== Test Case 1: Valid file copy ===\\n");
    processFile("/tmp/test_source", "valid.txt", "/tmp/test_dest", "output1.txt");
    
    printf("\\n=== Test Case 2: Path traversal attempt ===\\n");
    processFile("/tmp/test_source", "../etc/passwd", "/tmp/test_dest", "output2.txt");
    
    printf("\\n=== Test Case 3: Absolute path attempt ===\\n");
    processFile("/tmp/test_source", "/etc/shadow", "/tmp/test_dest", "output3.txt");
    
    printf("\\n=== Test Case 4: Nonexistent file ===\\n");
    processFile("/tmp/test_source", "nonexistent.txt", "/tmp/test_dest", "output4.txt");
    
    printf("\\n=== Test Case 5: Empty filename ===\\n");
    processFile("/tmp/test_source", "", "/tmp/test_dest", "output5.txt");
    
    return 0;
}
