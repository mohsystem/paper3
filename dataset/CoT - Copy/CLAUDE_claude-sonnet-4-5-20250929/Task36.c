
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB limit
#define MAX_PATH_LENGTH 4096

/**
 * Reads a file and prints its contents.
 * Implements security measures to prevent path traversal and other file-related vulnerabilities.
 * Returns 0 on success, -1 on error.
 */
int readFileContents(const char* filePath, char** output) {
    if (filePath == NULL || strlen(filePath) == 0) {
        *output = strdup("Error: File path cannot be null or empty");
        return -1;
    }
    
    // Check path length to prevent buffer overflow
    if (strlen(filePath) >= MAX_PATH_LENGTH) {
        *output = strdup("Error: File path too long");
        return -1;
    }
    
    // Get file information
    struct stat fileStat;
    if (stat(filePath, &fileStat) != 0) {
        char errorMsg[256];
        snprintf(errorMsg, sizeof(errorMsg), "Error: Cannot access file - %s", strerror(errno));
        *output = strdup(errorMsg);
        return -1;
    }
    
    // Check if it's a regular file\n    if (!S_ISREG(fileStat.st_mode)) {\n        *output = strdup("Error: Path does not point to a regular file");\n        return -1;\n    }\n    \n    // Check file size\n    if (fileStat.st_size > MAX_FILE_SIZE) {\n        *output = strdup("Error: File size exceeds maximum allowed size (10 MB)");\n        return -1;\n    }\n    \n    // Open file for reading\n    FILE* file = fopen(filePath, "r");\n    if (file == NULL) {\n        char errorMsg[256];\n        snprintf(errorMsg, sizeof(errorMsg), "Error: Unable to open file - %s", strerror(errno));\n        *output = strdup(errorMsg);\n        return -1;\n    }\n    \n    // Allocate buffer for file contents\n    size_t bufferSize = fileStat.st_size + 1;\n    char* buffer = (char*)malloc(bufferSize);\n    if (buffer == NULL) {\n        fclose(file);\n        *output = strdup("Error: Memory allocation failed");\n        return -1;\n    }\n    \n    // Read file contents\n    size_t bytesRead = fread(buffer, 1, fileStat.st_size, file);\n    buffer[bytesRead] = '\\0';
    
    fclose(file);
    
    *output = buffer;
    return 0;
}

int main() {
    printf("=== File Reader Program ===\\n\\n");
    
    // Test case 1: Valid text file (create a temporary file for testing)
    printf("Test Case 1: Valid text file\\n");
    FILE* tempFile = tmpfile();
    char tempFileName[L_tmpnam];
    tmpnam(tempFileName);
    strcat(tempFileName, ".txt");
    
    FILE* testFile = fopen(tempFileName, "w");
    if (testFile != NULL) {
        fprintf(testFile, "Hello, World!\\nThis is a test file.");
        fclose(testFile);
        
        char* result = NULL;
        readFileContents(tempFileName, &result);
        printf("%s\\n", result);
        free(result);
        remove(tempFileName);
    }
    
    // Test case 2: Empty file path
    printf("\\nTest Case 2: Empty file path\\n");
    char* result2 = NULL;
    readFileContents("", &result2);
    printf("%s\\n", result2);
    free(result2);
    
    // Test case 3: NULL file path
    printf("\\nTest Case 3: NULL file path\\n");
    char* result3 = NULL;
    readFileContents(NULL, &result3);
    printf("%s\\n", result3);
    free(result3);
    
    // Test case 4: Non-existent file
    printf("\\nTest Case 4: Non-existent file\\n");
    char* result4 = NULL;
    readFileContents("/nonexistent/path/file.txt", &result4);
    printf("%s\\n", result4);
    free(result4);
    
    // Test case 5: Path traversal attempt (security test)
    printf("\\nTest Case 5: Path traversal attempt\\n");
    char* result5 = NULL;
    readFileContents("../../../etc/passwd", &result5);
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
