
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#define SAFE_DIRECTORY "/safe/"
#define MAX_BUFFER 4096

char* readSafeFile(const char* filename) {
    static char result[MAX_BUFFER];
    char fullPath[PATH_MAX];
    char safePath[PATH_MAX];
    char resolvedPath[PATH_MAX];
    FILE* file;
    size_t bytesRead;
    
    if (filename == NULL || strlen(filename) == 0) {
        snprintf(result, MAX_BUFFER, "Error: Filename cannot be empty");
        return result;
    }
    
    // Get canonical path of safe directory
    if (realpath(SAFE_DIRECTORY, safePath) == NULL) {
        snprintf(result, MAX_BUFFER, "Error: Safe directory not accessible");
        return result;
    }
    
    // Construct full path
    snprintf(fullPath, PATH_MAX, "%s/%s", safePath, filename);
    
    // Resolve to canonical path
    if (realpath(fullPath, resolvedPath) == NULL) {
        snprintf(result, MAX_BUFFER, "Error: File not found or not readable");
        return result;
    }
    
    // Verify the resolved path is within safe directory
    if (strncmp(resolvedPath, safePath, strlen(safePath)) != 0) {
        snprintf(result, MAX_BUFFER, "Error: Access denied - file outside safe directory");
        return result;
    }
    
    // Open and read file
    file = fopen(resolvedPath, "r");
    if (file == NULL) {
        snprintf(result, MAX_BUFFER, "Error: Unable to open file");
        return result;
    }
    
    bytesRead = fread(result, 1, MAX_BUFFER - 1, file);
    result[bytesRead] = '\\0';
    
    fclose(file);
    return result;
}

int main() {
    // Test cases
    printf("Test 1 - Normal file:\\n%s\\n\\n", readSafeFile("test.txt"));
    
    printf("Test 2 - Path traversal attempt:\\n%s\\n\\n", readSafeFile("../etc/passwd"));
    
    printf("Test 3 - Empty filename:\\n%s\\n\\n", readSafeFile(""));
    
    printf("Test 4 - NULL filename:\\n%s\\n\\n", readSafeFile(NULL));
    
    printf("Test 5 - Non-existent file:\\n%s\\n\\n", readSafeFile("nonexistent.txt"));
    
    return 0;
}
