
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

char* readFileFromSafeDirectory(const char* filename) {
    char* result = NULL;
    char safeDir[PATH_MAX];
    char filePath[PATH_MAX];
    char resolvedPath[PATH_MAX];
    FILE* file = NULL;
    
    // Define the safe directory
    strcpy(safeDir, "/safe/");
    
    // Construct file path (basic concatenation with size check)
    if (strlen(safeDir) + strlen(filename) >= PATH_MAX) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Path too long");
        return result;
    }
    
    snprintf(filePath, PATH_MAX, "%s%s", safeDir, filename);
    
    // Resolve to absolute path
    if (realpath(filePath, resolvedPath) == NULL) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Cannot resolve path");
        return result;
    }
    
    // Security check: ensure resolved path starts with safe directory
    if (strncmp(resolvedPath, safeDir, strlen(safeDir)) != 0) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Access denied - path traversal detected");
        return result;
    }
    
    // Open and read file
    file = fopen(resolvedPath, "r");
    if (file == NULL) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Unable to open file");
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate buffer and read
    result = (char*)malloc(fileSize + 1);
    if (result != NULL) {
        size_t bytesRead = fread(result, 1, fileSize, file);
        result[bytesRead] = '\\0';
    }
    
    fclose(file);
    return result;
}

int main() {
    char* content;
    
    // Test case 1: Normal file
    printf("Test 1 - Normal file:\\n");
    content = readFileFromSafeDirectory("test.txt");
    printf("%s\\n", content);
    free(content);
    
    // Test case 2: Path traversal attempt
    printf("\\nTest 2 - Path traversal attempt:\\n");
    content = readFileFromSafeDirectory("../etc/passwd");
    printf("%s\\n", content);
    free(content);
    
    // Test case 3: Another path traversal
    printf("\\nTest 3 - Another traversal:\\n");
    content = readFileFromSafeDirectory("../../secret.txt");
    printf("%s\\n", content);
    free(content);
    
    // Test case 4: Subdirectory file
    printf("\\nTest 4 - Subdirectory:\\n");
    content = readFileFromSafeDirectory("subdir/file.txt");
    printf("%s\\n", content);
    free(content);
    
    // Test case 5: Non-existent file
    printf("\\nTest 5 - Non-existent file:\\n");
    content = readFileFromSafeDirectory("nonexistent.txt");
    printf("%s\\n", content);
    free(content);
    
    return 0;
}
