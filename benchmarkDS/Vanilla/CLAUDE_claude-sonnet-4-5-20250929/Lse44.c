
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFileFromSafeDirectory(const char* filename) {
    char* result = (char*)malloc(10000 * sizeof(char));
    if (result == NULL) {
        return NULL;
    }
    
    // Construct the safe directory path
    char filePath[512];
    snprintf(filePath, sizeof(filePath), "/safe/%s", filename);
    
    // Basic security check for path traversal
    if (strstr(filename, "..") != NULL) {
        strcpy(result, "Error: Access denied - path traversal attempt detected");
        return result;
    }
    
    // Read the file content
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        strcpy(result, "Error reading file: Could not open file");
        return result;
    }
    
    size_t offset = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        size_t len = strlen(buffer);
        if (offset + len < 10000) {
            strcpy(result + offset, buffer);
            offset += len;
        }
    }
    
    fclose(file);
    return result;
}

int main() {
    char* content;
    
    // Test cases
    printf("Test Case 1: Reading 'document.txt'\\n");
    content = readFileFromSafeDirectory("document.txt");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest Case 2: Reading 'data.txt'\\n");
    content = readFileFromSafeDirectory("data.txt");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest Case 3: Path traversal attempt '../etc/passwd'\\n");
    content = readFileFromSafeDirectory("../etc/passwd");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest Case 4: Reading subdirectory file 'subdir/file.txt'\\n");
    content = readFileFromSafeDirectory("subdir/file.txt");
    printf("%s\\n", content);
    free(content);
    
    printf("\\nTest Case 5: Non-existent file 'nonexistent.txt'\\n");
    content = readFileFromSafeDirectory("nonexistent.txt");
    printf("%s\\n", content);
    free(content);
    
    return 0;
}
