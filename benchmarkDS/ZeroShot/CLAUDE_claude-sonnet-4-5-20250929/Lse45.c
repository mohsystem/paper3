
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <libgen.h>

#define SAFE_DIRECTORY "/var/safe_files/"
#define MAX_PATH 4096
#define MAX_CONTENT 10000

char* readFileFromSafeDirectory(const char* filename, char* result) {
    char safePath[MAX_PATH];
    char filePath[MAX_PATH];
    char resolvedPath[MAX_PATH];
    char* baseFilename;
    char filenameCopy[MAX_PATH];
    FILE* file;
    size_t bytesRead;
    
    // Initialize result
    result[0] = '\\0';
    
    // Copy filename for basename extraction
    strncpy(filenameCopy, filename, MAX_PATH - 1);
    filenameCopy[MAX_PATH - 1] = '\\0';
    
    // Sanitize the filename to prevent path traversal attacks
    baseFilename = basename(filenameCopy);
    
    // Construct the full path within the safe directory
    snprintf(filePath, MAX_PATH, "%s%s", SAFE_DIRECTORY, baseFilename);
    
    // Resolve the real path
    if (realpath(filePath, resolvedPath) == NULL) {
        strcpy(result, "Error: File not found or cannot resolve path");
        return result;
    }
    
    // Resolve safe directory path
    if (realpath(SAFE_DIRECTORY, safePath) == NULL) {
        strcpy(result, "Error: Cannot resolve safe directory");
        return result;
    }
    
    // Verify the file is still within the safe directory
    if (strncmp(resolvedPath, safePath, strlen(safePath)) != 0) {
        strcpy(result, "Error: Path traversal attempt detected");
        return result;
    }
    
    // Open and read the file
    file = fopen(resolvedPath, "r");
    if (file == NULL) {
        strcpy(result, "Error: Cannot open file");
        return result;
    }
    
    bytesRead = fread(result, 1, MAX_CONTENT - 1, file);
    result[bytesRead] = '\\0';
    fclose(file);
    
    return result;
}

int main() {
    char result[MAX_CONTENT];
    
    // Test cases
    printf("Test 1: %s\\n", readFileFromSafeDirectory("document.txt", result));
    printf("Test 2: %s\\n", readFileFromSafeDirectory("data.json", result));
    printf("Test 3: %s\\n", readFileFromSafeDirectory("../etc/passwd", result));
    printf("Test 4: %s\\n", readFileFromSafeDirectory("nonexistent.txt", result));
    printf("Test 5: %s\\n", readFileFromSafeDirectory("config.ini", result));
    
    return 0;
}
