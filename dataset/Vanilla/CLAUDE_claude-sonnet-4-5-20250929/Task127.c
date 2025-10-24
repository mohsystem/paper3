
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/types.h>
#endif

int fileExists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

int isDirectory(const char* path) {
    struct stat buffer;
    if (stat(path, &buffer) != 0) {
        return 0;
    }
    return S_ISDIR(buffer.st_mode);
}

int createDirectory(const char* path) {
    return mkdir(path, 0755) == 0 || errno == EEXIST;
}

void getFileName(const char* path, char* fileName) {
    const char* lastSlash = strrchr(path, '/');
    const char* lastBackslash = strrchr(path, '\\\\');\n    const char* start = path;\n    \n    if (lastSlash != NULL && (lastBackslash == NULL || lastSlash > lastBackslash)) {\n        start = lastSlash + 1;\n    } else if (lastBackslash != NULL) {\n        start = lastBackslash + 1;\n    }\n    \n    strcpy(fileName, start);\n}\n\nchar* readAndWriteFile(const char* sourceFilePath, const char* tempDirectory) {\n    FILE* reader = NULL;\n    FILE* writer = NULL;\n    char* tempFilePath = NULL;\n    char line[1024];\n    char fileName[256];\n    \n    // Validate input parameters\n    if (sourceFilePath == NULL || strlen(sourceFilePath) == 0) {\n        fprintf(stderr, "Invalid argument: Source file path cannot be NULL or empty\\n");\n        return NULL;\n    }\n    if (tempDirectory == NULL || strlen(tempDirectory) == 0) {\n        fprintf(stderr, "Invalid argument: Temp directory cannot be NULL or empty\\n");\n        return NULL;\n    }\n    \n    // Check if source file exists\n    if (!fileExists(sourceFilePath)) {\n        fprintf(stderr, "File not found error: Source file does not exist: %s\\n", sourceFilePath);\n        return NULL;\n    }\n    if (isDirectory(sourceFilePath)) {\n        fprintf(stderr, "Invalid argument: Source path is not a file: %s\\n", sourceFilePath);\n        return NULL;\n    }\n    \n    // Create temp directory if it doesn't exist
    if (!fileExists(tempDirectory)) {
        if (!createDirectory(tempDirectory)) {
            fprintf(stderr, "IO error: Failed to create temp directory: %s\\n", tempDirectory);
            return NULL;
        }
    }
    
    // Create temp file path
    getFileName(sourceFilePath, fileName);
    tempFilePath = (char*)malloc(512 * sizeof(char));
    if (tempFilePath == NULL) {
        fprintf(stderr, "Memory allocation error\\n");
        return NULL;
    }
    snprintf(tempFilePath, 512, "%s/temp_%ld_%s", tempDirectory, (long)time(NULL), fileName);
    
    // Open source file
    reader = fopen(sourceFilePath, "r");
    if (reader == NULL) {
        fprintf(stderr, "IO error: Cannot open source file: %s\\n", sourceFilePath);
        free(tempFilePath);
        return NULL;
    }
    
    // Open temp file
    writer = fopen(tempFilePath, "w");
    if (writer == NULL) {
        fprintf(stderr, "IO error: Cannot create temp file: %s\\n", tempFilePath);
        fclose(reader);
        free(tempFilePath);
        return NULL;
    }
    
    // Copy content
    while (fgets(line, sizeof(line), reader) != NULL) {
        fputs(line, writer);
    }
    
    // Close files
    fclose(reader);
    fclose(writer);
    
    return tempFilePath;
}

int main() {
    printf("Testing File Read and Write with Exception Handling\\n\\n");
    
    // Test Case 1: Valid file and temp directory
    printf("Test Case 1: Valid file and temp directory\\n");
    {
        FILE* testFile = fopen("test1.txt", "w");
        fprintf(testFile, "Hello World!\\nThis is test file 1.\\n");
        fclose(testFile);
        
        char* result = readAndWriteFile("test1.txt", "./temp");
        printf("Result: %s\\n", result ? result : "Failed");
        if (result) free(result);
        remove("test1.txt");
    }
    printf("\\n");
    
    // Test Case 2: Non-existent source file
    printf("Test Case 2: Non-existent source file\\n");
    char* result2 = readAndWriteFile("nonexistent.txt", "./temp");
    printf("Result: %s\\n", result2 ? result2 : "Failed (Expected)");
    if (result2) free(result2);
    printf("\\n");
    
    // Test Case 3: NULL source file path
    printf("Test Case 3: NULL source file path\\n");
    char* result3 = readAndWriteFile(NULL, "./temp");
    printf("Result: %s\\n", result3 ? result3 : "Failed (Expected)");
    if (result3) free(result3);
    printf("\\n");
    
    // Test Case 4: Empty source file path
    printf("Test Case 4: Empty source file path\\n");
    char* result4 = readAndWriteFile("", "./temp");
    printf("Result: %s\\n", result4 ? result4 : "Failed (Expected)");
    if (result4) free(result4);
    printf("\\n");
    
    // Test Case 5: Valid file with special characters
    printf("Test Case 5: Valid file with special characters\\n");
    {
        FILE* testFile = fopen("test5.txt", "w");
        fprintf(testFile, "Special chars: @#$%%^&*()\\n");
        fclose(testFile);
        
        char* result5 = readAndWriteFile("test5.txt", "./temp");
        printf("Result: %s\\n", result5 ? result5 : "Failed");
        if (result5) free(result5);
        remove("test5.txt");
    }
    printf("\\n");
    
    return 0;
}
