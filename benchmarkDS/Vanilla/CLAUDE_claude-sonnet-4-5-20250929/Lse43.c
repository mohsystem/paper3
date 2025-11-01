
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readFileFromSafeDirectory(const char* filename) {
    char safePath[512];
    snprintf(safePath, sizeof(safePath), "/safe/%s", filename);
    
    FILE* file = fopen(safePath, "r");
    if (file == NULL) {
        char* error = (char*)malloc(100);
        strcpy(error, "Error reading file: Unable to open file");
        return error;
    }
    
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* content = (char*)malloc(fileSize + 1);
    if (content == NULL) {
        fclose(file);
        char* error = (char*)malloc(100);
        strcpy(error, "Error: Memory allocation failed");
        return error;
    }
    
    fread(content, 1, fileSize, file);
    content[fileSize] = '\\0';
    
    fclose(file);
    return content;
}

int main() {
    // Test cases
    const char* testFiles[] = {
        "test1.txt",
        "test2.txt",
        "data.txt",
        "config.txt",
        "info.txt"
    };
    
    int numTests = sizeof(testFiles) / sizeof(testFiles[0]);
    
    for (int i = 0; i < numTests; i++) {
        printf("Reading file: %s\\n", testFiles[i]);
        char* content = readFileFromSafeDirectory(testFiles[i]);
        printf("Content: %s\\n", content);
        printf("---\\n");
        free(content);
    }
    
    return 0;
}
