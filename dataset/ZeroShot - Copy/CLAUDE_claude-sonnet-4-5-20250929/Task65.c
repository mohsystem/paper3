
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_FILE_SIZE (10 * 1024 * 1024)
#define MAX_BUFFER_SIZE 1024

int isValidFilename(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        return 0;
    }
    
    for (size_t i = 0; i < strlen(filename); i++) {
        char c = filename[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return 0;
        }
    }
    return 1;
}

char* readFileSecurely(const char* filename) {
    static char errorMsg[256];
    static char* content = NULL;
    
    if (content != NULL) {
        free(content);
        content = NULL;
    }
    
    if (filename == NULL || strlen(filename) == 0) {
        strcpy(errorMsg, "Error: Filename cannot be null or empty");
        return errorMsg;
    }
    
    // Validate filename format (no path traversal)
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL || 
        strchr(filename, '\\\\') != NULL) {\n        strcpy(errorMsg, "Error: Invalid filename format - path traversal detected");\n        return errorMsg;\n    }\n    \n    // Validate filename characters\n    if (!isValidFilename(filename)) {\n        strcpy(errorMsg, "Error: Filename contains invalid characters");\n        return errorMsg;\n    }\n    \n    // Check if file exists and get stats\n    struct stat fileStat;\n    if (stat(filename, &fileStat) != 0) {\n        snprintf(errorMsg, sizeof(errorMsg), "Error: File does not exist: %s", filename);\n        return errorMsg;\n    }\n    \n    // Check if it's a regular file
    if (!S_ISREG(fileStat.st_mode)) {
        snprintf(errorMsg, sizeof(errorMsg), "Error: Path is not a regular file: %s", filename);
        return errorMsg;
    }
    
    // Check file size
    if (fileStat.st_size > MAX_FILE_SIZE) {
        strcpy(errorMsg, "Error: File size exceeds maximum allowed size");
        return errorMsg;
    }
    
    // Read file content
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        snprintf(errorMsg, sizeof(errorMsg), "Error: Failed to open file: %s", filename);
        return errorMsg;
    }
    
    content = (char*)malloc(fileStat.st_size + 1);
    if (content == NULL) {
        fclose(file);
        strcpy(errorMsg, "Error: Memory allocation failed");
        return errorMsg;
    }
    
    size_t bytesRead = fread(content, 1, fileStat.st_size, file);
    content[bytesRead] = '\\0';
    fclose(file);
    
    return content;
}

int main() {
    printf("=== Test Case 1: Valid file ===\\n");
    FILE* testFile = fopen("test1.txt", "w");
    if (testFile != NULL) {
        fprintf(testFile, "Hello World!\\n");
        fprintf(testFile, "This is a test file.\\n");
        fclose(testFile);
        printf("%s\\n", readFileSecurely("test1.txt"));
    } else {
        printf("Could not create test file\\n");
    }
    
    printf("\\n=== Test Case 2: Non-existent file ===\\n");
    printf("%s\\n", readFileSecurely("nonexistent.txt"));
    
    printf("\\n=== Test Case 3: Path traversal attempt ===\\n");
    printf("%s\\n", readFileSecurely("../etc/passwd"));
    
    printf("\\n=== Test Case 4: Invalid characters ===\\n");
    printf("%s\\n", readFileSecurely("test@#$.txt"));
    
    printf("\\n=== Test Case 5: Empty filename ===\\n");
    printf("%s\\n", readFileSecurely(""));
    
    return 0;
}
