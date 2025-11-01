
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define SAFE_DIRECTORY "/safe/"
#define MAX_PATH_LEN 4096
#define MAX_CONTENT_LEN 65536

char* readFileFromSafeDirectory(const char* requestedFile) {
    char* result = (char*)malloc(MAX_CONTENT_LEN);
    if (!result) {
        return NULL;
    }
    
    char safePath[MAX_PATH_LEN];
    char requestedPath[MAX_PATH_LEN];
    char resolvedPath[MAX_PATH_LEN];
    
    // Build the full path
    snprintf(requestedPath, MAX_PATH_LEN, "%s%s", SAFE_DIRECTORY, requestedFile);
    
    // Resolve the real path
    if (realpath(requestedPath, resolvedPath) == NULL) {
        snprintf(result, MAX_CONTENT_LEN, "Error: File not found or inaccessible");
        return result;
    }
    
    // Get the real path of safe directory
    if (realpath(SAFE_DIRECTORY, safePath) == NULL) {
        snprintf(result, MAX_CONTENT_LEN, "Error: Safe directory not accessible");
        return result;
    }
    
    // Security check: ensure the resolved path is still within the safe directory
    if (strncmp(resolvedPath, safePath, strlen(safePath)) != 0) {
        snprintf(result, MAX_CONTENT_LEN, "Error: Access denied - Path traversal attempt detected");
        return result;
    }
    
    // Read the file
    FILE* file = fopen(resolvedPath, "r");
    if (!file) {
        snprintf(result, MAX_CONTENT_LEN, "Error: Unable to open file");
        return result;
    }
    
    size_t bytesRead = fread(result, 1, MAX_CONTENT_LEN - 1, file);
    result[bytesRead] = '\\0';
    fclose(file);
    
    return result;
}

int main() {
    char* content;
    
    // Test cases
    printf("Test 1 - Normal file:\\n");
    content = readFileFromSafeDirectory("test.txt");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest 2 - Path traversal attempt:\\n");
    content = readFileFromSafeDirectory("../../../etc/passwd");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest 3 - Another path traversal:\\n");
    content = readFileFromSafeDirectory("..\\\\..\\\\windows\\\\system32\\\\config\\\\sam");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest 4 - Non-existent file:\\n");
    content = readFileFromSafeDirectory("nonexistent.txt");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest 5 - Subdirectory file:\\n");
    content = readFileFromSafeDirectory("subdir/file.txt");
    printf("%s\\n", content);
    free(content);
    
    return 0;
}
