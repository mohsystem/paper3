
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

/* Maximum file size to prevent excessive memory usage (10 MB) */
#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_PATH_LENGTH 4096

/* Function to clear sensitive data securely */
void secure_clear(void* ptr, size_t size) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (size--) {
        *p++ = 0;
    }
}

/* Function to validate file path - rejects traversal attempts and absolute paths */
int validatePath(const char* path) {
    /* Null pointer check */
    if (path == NULL) {
        return 0;
    }
    
    /* Length validation */
    size_t len = strlen(path);
    if (len == 0 || len > MAX_PATH_LENGTH) {
        return 0;
    }
    
    /* Check for null bytes in path (injection attempt) */
    if (memchr(path, '\\0', len) != path + len) {
        return 0;
    }
    
    /* Reject absolute paths */
    if (path[0] == '/') {
        return 0;
    }
    
    /* Reject path traversal attempts */
    if (strstr(path, "..") != NULL) {
        return 0;
    }
    
    /* Reject paths with null bytes */
    for (size_t i = 0; i < len; i++) {
        if (path[i] == '\\0' && i < len - 1) {
            return 0;
        }
    }
    
    return 1;
}

/* Function to read file content securely with bounds checking */
int readFileContent(const char* filepath, char** outContent, size_t* outSize) {
    FILE* file = NULL;
    char* buffer = NULL;
    size_t fileSize = 0;
    size_t bytesRead = 0;
    
    /* Initialize output parameters */
    if (outContent == NULL || outSize == NULL) {
        fprintf(stderr, "Error: Invalid output parameters\\n");
        return 0;
    }
    *outContent = NULL;
    *outSize = 0;
    
    /* Validate filepath */
    if (!validatePath(filepath)) {
        fprintf(stderr, "Error: Invalid file path\\n");
        return 0;
    }
    
    /* Open file with exclusive access check using "r" mode */
    file = fopen(filepath, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file\\n");
        return 0;
    }
    
    /* Get file size with bounds checking */
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: Cannot seek to end of file\\n");
        fclose(file);
        return 0;
    }
    
    long fileSizeLong = ftell(file);
    if (fileSizeLong < 0) {
        fprintf(stderr, "Error: Cannot determine file size\\n");
        fclose(file);
        return 0;
    }
    
    /* Validate file size against maximum to prevent excessive memory usage */
    if ((unsigned long)fileSizeLong > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File too large (max %d bytes)\\n", MAX_FILE_SIZE);
        fclose(file);
        return 0;
    }
    
    fileSize = (size_t)fileSizeLong;
    
    /* Return to beginning */
    if (fseek(file, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Error: Cannot seek to beginning of file\\n");
        fclose(file);
        return 0;
    }
    
    /* Allocate buffer with size validation and null terminator space */
    if (fileSize > SIZE_MAX - 1) {
        fprintf(stderr, "Error: File size overflow\\n");
        fclose(file);
        return 0;
    }
    
    buffer = (char*)calloc(fileSize + 1, sizeof(char));
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        fclose(file);
        return 0;
    }
    
    /* Read file content with bounds checking */
    if (fileSize > 0) {
        bytesRead = fread(buffer, 1, fileSize, file);
        if (bytesRead != fileSize) {
            if (ferror(file)) {
                fprintf(stderr, "Error: Failed to read file content\\n");
                secure_clear(buffer, fileSize + 1);
                free(buffer);
                fclose(file);
                return 0;
            }
            /* Adjust size if partial read occurred at EOF */
            fileSize = bytesRead;
        }
    }
    
    /* Ensure null termination */
    buffer[fileSize] = '\\0';
    
    /* Close file */
    if (fclose(file) != 0) {
        fprintf(stderr, "Warning: Error closing file\\n");
    }
    
    /* Set output parameters */
    *outContent = buffer;
    *outSize = fileSize;
    
    return 1;
}

int main(int argc, char* argv[]) {
    char* content = NULL;
    size_t contentSize = 0;
    
    /* Validate argument count */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filepath>\\n", argc > 0 && argv != NULL ? argv[0] : "program");
        return 1;
    }
    
    /* Validate argv pointers */
    if (argv == NULL || argv[1] == NULL) {
        fprintf(stderr, "Error: Invalid arguments\\n");
        return 1;
    }
    
    /* Read file content */
    if (!readFileContent(argv[1], &content, &contentSize)) {
        return 1;
    }
    
    /* Output content to stdout - use fwrite for binary safety */
    if (content != NULL && contentSize > 0) {
        size_t written = fwrite(content, 1, contentSize, stdout);
        if (written != contentSize) {
            fprintf(stderr, "Error: Failed to write complete output\\n");
            secure_clear(content, contentSize);
            free(content);
            return 1;
        }
    }
    
    /* Clean up - clear and free memory */
    if (content != NULL) {
        secure_clear(content, contentSize);
        free(content);
        content = NULL;
    }
    
    return 0;
}

/* Test cases - compile with -DRUN_TESTS */
#ifdef RUN_TESTS
void runTests(void) {
    printf("Running test cases...\\n");
    
    /* Test 1: Valid file */
    FILE* f1 = fopen("test1.txt", "w");
    if (f1) {
        fprintf(f1, "Hello, World!");
        fclose(f1);
        
        char* content = NULL;
        size_t size = 0;
        if (readFileContent("test1.txt", &content, &size)) {
            printf("Test 1 passed: %s\\n", content);
            secure_clear(content, size);
            free(content);
        }
    }
    
    /* Test 2: Non-existent file */
    char* content2 = NULL;
    size_t size2 = 0;
    if (!readFileContent("nonexistent.txt", &content2, &size2)) {
        printf("Test 2 passed: Correctly handled non-existent file\\n");
    }
    
    /* Test 3: Path traversal attempt */
    if (!validatePath("../etc/passwd")) {
        printf("Test 3 passed: Blocked path traversal\\n");
    }
    
    /* Test 4: Empty path */
    if (!validatePath("")) {
        printf("Test 4 passed: Rejected empty path\\n");
    }
    
    /* Test 5: Valid multiline content */
    FILE* f5 = fopen("test5.txt", "w");
    if (f5) {
        fprintf(f5, "Line 1\\nLine 2\\nLine 3");
        fclose(f5);
        
        char* content5 = NULL;
        size_t size5 = 0;
        if (readFileContent("test5.txt", &content5, &size5)) {
            printf("Test 5 passed: Read multiline content\\n");
            secure_clear(content5, size5);
            free(content5);
        }
    }
}
#endif
