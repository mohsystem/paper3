
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

char* readFileFromSafeDirectory(const char* filename, const char* safeDirectory) {
    char safePath[PATH_MAX];
    char requestedPath[PATH_MAX];
    char fullPath[PATH_MAX];
    FILE* file;
    long fileSize;
    char* content;
    
    // Resolve safe directory to absolute path
    if (realpath(safeDirectory, safePath) == NULL) {
        char* error = malloc(100);
        snprintf(error, 100, "Error: Cannot resolve safe directory");
        return error;
    }
    
    // Build full path
    snprintf(fullPath, PATH_MAX, "%s/%s", safePath, filename);
    
    // Resolve requested file path
    if (realpath(fullPath, requestedPath) == NULL) {
        // File might not exist, try to check parent directory
        char* error = malloc(100);
        snprintf(error, 100, "Error: File not found");
        return error;
    }
    
    // Check if requested path starts with safe path
    if (strncmp(requestedPath, safePath, strlen(safePath)) != 0) {
        char* error = malloc(100);
        snprintf(error, 100, "Error: Access denied - file is outside safe directory");
        return error;
    }
    
    // Open and read file
    file = fopen(requestedPath, "r");
    if (file == NULL) {
        char* error = malloc(100);
        snprintf(error, 100, "Error: Cannot open file");
        return error;
    }
    
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    content = malloc(fileSize + 1);
    fread(content, 1, fileSize, file);
    content[fileSize] = '\\0';
    
    fclose(file);
    return content;
}

int main() {
    // Create a test safe directory and files
    const char* safeDir = "safe_directory";
    mkdir(safeDir, 0755);
    
    FILE* f1 = fopen("safe_directory/test1.txt", "w");
    fprintf(f1, "Content of test1");
    fclose(f1);
    
    FILE* f2 = fopen("safe_directory/test2.txt", "w");
    fprintf(f2, "Content of test2");
    fclose(f2);
    
    // Test case 1
    char* result1 = readFileFromSafeDirectory("test1.txt", safeDir);
    printf("Test 1: %s\\n", result1);
    free(result1);
    
    // Test case 2
    char* result2 = readFileFromSafeDirectory("test2.txt", safeDir);
    printf("Test 2: %s\\n", result2);
    free(result2);
    
    // Test case 3
    char* result3 = readFileFromSafeDirectory("nonexistent.txt", safeDir);
    printf("Test 3: %s\\n", result3);
    free(result3);
    
    // Test case 4
    char* result4 = readFileFromSafeDirectory("../test.txt", safeDir);
    printf("Test 4: %s\\n", result4);
    free(result4);
    
    // Test case 5
    char* result5 = readFileFromSafeDirectory("../../etc/passwd", safeDir);
    printf("Test 5: %s\\n", result5);
    free(result5);
    
    return 0;
}
