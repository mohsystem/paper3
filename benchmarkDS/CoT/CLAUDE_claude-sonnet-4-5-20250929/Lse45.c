
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>

#define SAFE_DIRECTORY "/var/safe_files/"
#define MAX_FILENAME 256
#define MAX_CONTENT 65536

char* readFileFromSafeDirectory(const char* filename) {
    static char result[MAX_CONTENT];
    char sanitizedFilename[MAX_FILENAME];
    char fullPath[PATH_MAX];
    char resolvedPath[PATH_MAX];
    char safeDirResolved[PATH_MAX];
    FILE* file;
    struct stat statbuf;
    size_t bytesRead;
    
    // Input validation
    if (filename == NULL || strlen(filename) == 0) {
        strcpy(result, "Error: Invalid filename");
        return result;
    }
    
    // Sanitize filename - remove path separators
    int j = 0;
    for (int i = 0; filename[i] != '\\0' && j < MAX_FILENAME - 1; i++) {
        if (filename[i] != '/' && filename[i] != '\\\\') {\n            sanitizedFilename[j++] = filename[i];\n        }\n    }\n    sanitizedFilename[j] = '\\0';\n    \n    // Resolve safe directory canonical path\n    if (realpath(SAFE_DIRECTORY, safeDirResolved) == NULL) {\n        strcpy(result, "Error: Safe directory not accessible");\n        return result;\n    }\n    \n    // Build full path\n    snprintf(fullPath, PATH_MAX, "%s/%s", safeDirResolved, sanitizedFilename);\n    \n    // Resolve requested file canonical path\n    if (realpath(fullPath, resolvedPath) == NULL) {\n        strcpy(result, "Error: File not found");\n        return result;\n    }\n    \n    // Security check: ensure the resolved path is within safe directory\n    if (strncmp(resolvedPath, safeDirResolved, strlen(safeDirResolved)) != 0) {\n        strcpy(result, "Error: Access denied - file outside safe directory");\n        return result;\n    }\n    \n    // Check if it's a regular file
    if (stat(resolvedPath, &statbuf) != 0) {
        strcpy(result, "Error: Cannot access file");
        return result;
    }
    
    if (!S_ISREG(statbuf.st_mode)) {
        strcpy(result, "Error: Not a regular file");
        return result;
    }
    
    // Read file content
    file = fopen(resolvedPath, "r");
    if (file == NULL) {
        strcpy(result, "Error: Unable to open file");
        return result;
    }
    
    bytesRead = fread(result, 1, MAX_CONTENT - 1, file);
    result[bytesRead] = '\\0';
    fclose(file);
    
    return result;
}

int main() {
    // Test cases
    printf("Test 1 - Valid file:\\n%s\\n", 
           readFileFromSafeDirectory("document.txt"));
    
    printf("\\nTest 2 - Path traversal attempt:\\n%s\\n", 
           readFileFromSafeDirectory("../etc/passwd"));
    
    printf("\\nTest 3 - Absolute path attempt:\\n%s\\n", 
           readFileFromSafeDirectory("/etc/passwd"));
    
    printf("\\nTest 4 - NULL filename:\\n%s\\n", 
           readFileFromSafeDirectory(NULL));
    
    printf("\\nTest 5 - Non-existent file:\\n%s\\n", 
           readFileFromSafeDirectory("nonexistent.txt"));
    
    return 0;
}
