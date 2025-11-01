
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <libgen.h>

/**
 * Securely reads a file from the /safe/ directory
 * @param filename The name of the file to read (without path)
 * @return The content of the file as a dynamically allocated string, or error message
 *         Caller is responsible for freeing the returned string
 */
char* readSafeFile(const char* filename) {
    char* result = NULL;
    FILE* file = NULL;
    char safePath[PATH_MAX];
    char* sanitizedFilename = NULL;
    char filenameCopy[PATH_MAX];
    
    // Validate filename
    if (filename == NULL || strlen(filename) == 0) {
        result = (char*)malloc(50);
        strcpy(result, "Error: Filename cannot be empty");
        return result;
    }
    
    // Create a copy for basename (it may modify the string)
    strncpy(filenameCopy, filename, PATH_MAX - 1);
    filenameCopy[PATH_MAX - 1] = '\\0';
    
    // Extract just the filename (remove path components)
    sanitizedFilename = basename(filenameCopy);
    
    // Construct the safe path
    snprintf(safePath, PATH_MAX, "/safe/%s", sanitizedFilename);
    
    // Additional check: ensure no ".." remains in path
    if (strstr(safePath, "..") != NULL) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Access denied - Path traversal attempt detected");
        return result;
    }
    
    // Verify the path starts with /safe/
    if (strncmp(safePath, "/safe/", 6) != 0) {
        result = (char*)malloc(100);
        strcpy(result, "Error: Access denied - Path traversal attempt detected");
        return result;
    }
    
    // Try to open the file
    file = fopen(safePath, "r");
    if (file == NULL) {
        result = (char*)malloc(50);
        strcpy(result, "Error: Unable to open file");
        return result;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory and read file
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
    
    // Test cases
    printf("Test Case 1: Normal file\\n");
    content = readSafeFile("test.txt");
    printf("%s\\n\\n", content);
    free(content);
    
    printf("Test Case 2: Path traversal attempt with ../\\n");
    content = readSafeFile("../etc/passwd");
    printf("%s\\n\\n", content);
    free(content);
    
    printf("Test Case 3: Absolute path attempt\\n");
    content = readSafeFile("/etc/passwd");
    printf("%s\\n\\n", content);
    free(content);
    
    printf("Test Case 4: Empty filename\\n");
    content = readSafeFile("");
    printf("%s\\n\\n", content);
    free(content);
    
    printf("Test Case 5: NULL filename\\n");
    content = readSafeFile(NULL);
    printf("%s\\n", content);
    free(content);
    
    return 0;
}
