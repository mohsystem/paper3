
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#define BUFFER_SIZE 8192
#define MAX_PATH 4096

char* copyFileToTemp(const char* sourceFilePath) {
    if (sourceFilePath == NULL || strlen(sourceFilePath) == 0) {
        fprintf(stderr, "Error: Source file path cannot be NULL or empty\\n");
        return NULL;
    }
    
    // Check if file exists and is readable
    struct stat fileStat;
    if (stat(sourceFilePath, &fileStat) != 0) {
        fprintf(stderr, "Error: Source file does not exist: %s\\n", sourceFilePath);
        return NULL;
    }
    
    if (!S_ISREG(fileStat.st_mode)) {
        fprintf(stderr, "Error: Source path is not a regular file: %s\\n", sourceFilePath);
        return NULL;
    }
    
    if (access(sourceFilePath, R_OK) != 0) {
        fprintf(stderr, "Error: Source file is not readable: %s\\n", sourceFilePath);
        return NULL;
    }
    
    // Create temporary file
    char tempTemplate[] = "/tmp/secure_temp_XXXXXX";
    int tempFd = mkstemp(tempTemplate);
    
    if (tempFd == -1) {
        fprintf(stderr, "Error: Failed to create temporary file: %s\\n", strerror(errno));
        return NULL;
    }
    
    char* tempPath = (char*)malloc(strlen(tempTemplate) + 1);
    if (tempPath == NULL) {
        close(tempFd);
        unlink(tempTemplate);
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    strcpy(tempPath, tempTemplate);
    
    // Open source file
    FILE* sourceFile = fopen(sourceFilePath, "rb");
    if (sourceFile == NULL) {
        close(tempFd);
        unlink(tempPath);
        free(tempPath);
        fprintf(stderr, "Error: Failed to open source file: %s\\n", strerror(errno));
        return NULL;
    }
    
    // Open temporary file for writing
    FILE* tempFile = fdopen(tempFd, "wb");
    if (tempFile == NULL) {
        fclose(sourceFile);
        close(tempFd);
        unlink(tempPath);
        free(tempPath);
        fprintf(stderr, "Error: Failed to open temporary file for writing: %s\\n", strerror(errno));
        return NULL;
    }
    
    // Copy content
    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;
    
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0) {
        size_t bytesWritten = fwrite(buffer, 1, bytesRead, tempFile);
        if (bytesWritten != bytesRead) {
            fclose(sourceFile);
            fclose(tempFile);
            unlink(tempPath);
            free(tempPath);
            fprintf(stderr, "Error: Failed to write to temporary file\\n");
            return NULL;
        }
    }
    
    if (ferror(sourceFile)) {
        fclose(sourceFile);
        fclose(tempFile);
        unlink(tempPath);
        free(tempPath);
        fprintf(stderr, "Error: Failed to read from source file\\n");
        return NULL;
    }
    
    fclose(sourceFile);
    fclose(tempFile);
    
    return tempPath;
}

int main() {
    // Test case 1: Valid file
    printf("Test 1: Valid file\\n");
    {
        char testTemplate[] = "/tmp/test1_XXXXXX";
        int testFd = mkstemp(testTemplate);
        if (testFd != -1) {
            write(testFd, "Test content 1", 14);
            close(testFd);
            
            char* result = copyFileToTemp(testTemplate);
            if (result != NULL) {
                printf("Success: Copied to %s\\n", result);
                unlink(testTemplate);
                unlink(result);
                free(result);
            }
        }
    }
    
    // Test case 2: Non-existent file
    printf("\\nTest 2: Non-existent file\\n");
    {
        char* result = copyFileToTemp("/non/existent/file.txt");
        if (result == NULL) {
            printf("Expected error occurred\\n");
        }
    }
    
    // Test case 3: NULL input
    printf("\\nTest 3: NULL input\\n");
    {
        char* result = copyFileToTemp(NULL);
        if (result == NULL) {
            printf("Expected error occurred\\n");
        }
    }
    
    // Test case 4: Empty string
    printf("\\nTest 4: Empty string\\n");
    {
        char* result = copyFileToTemp("");
        if (result == NULL) {
            printf("Expected error occurred\\n");
        }
    }
    
    // Test case 5: Large file
    printf("\\nTest 5: Large file\\n");
    {
        char testTemplate5[] = "/tmp/test5_XXXXXX";
        int testFd5 = mkstemp(testTemplate5);
        if (testFd5 != -1) {
            unsigned char largeBuffer[100000];
            for (int i = 0; i < 100000; i++) {
                largeBuffer[i] = (unsigned char)(i % 256);
            }
            write(testFd5, largeBuffer, 100000);
            close(testFd5);
            
            char* result = copyFileToTemp(testTemplate5);
            if (result != NULL) {
                printf("Success: Large file copied to %s\\n", result);
                unlink(testTemplate5);
                unlink(result);
                free(result);
            }
        }
    }
    
    return 0;
}
