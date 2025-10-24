#define _GNU_SOURCE // For realpath on Linux
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define BUFFER_SIZE 4096

/**
 * @brief Reads a file from a base directory and writes its content to a new temporary file.
 * This function is secured against path traversal attacks.
 *
 * @param baseDir The trusted base directory.
 * @param fileName The name of the file to be read (untrusted).
 * @return A dynamically allocated string with the path to the temporary file,
 *         or NULL if an error occurred. The caller must free the returned string.
 */
char* copyFileToTemp(const char* baseDir, const char* fileName) {
    if (strchr(fileName, '/') != NULL || strchr(fileName, '\\') != NULL) {
        fprintf(stderr, "Error: File name cannot contain path separators.\n");
        return NULL;
    }

    char inputFilePath[PATH_MAX];
    int len = snprintf(inputFilePath, sizeof(inputFilePath), "%s/%s", baseDir, fileName);
    if (len < 0 || (size_t)len >= sizeof(inputFilePath)) {
        fprintf(stderr, "Error: Path construction failed or buffer is too small.\n");
        return NULL;
    }

    char resolvedBase[PATH_MAX];
    char resolvedInput[PATH_MAX];

    if (realpath(baseDir, resolvedBase) == NULL) {
        perror("Error resolving base path");
        return NULL;
    }
    
    if (realpath(inputFilePath, resolvedInput) == NULL) {
        perror("Error resolving input file path");
        return NULL;
    }
    
    size_t baseLen = strlen(resolvedBase);
    if (strncmp(resolvedBase, resolvedInput, baseLen) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected. Access denied.\n");
        return NULL;
    }
    if (resolvedInput[baseLen] != '/' && resolvedInput[baseLen] != '\0') {
        fprintf(stderr, "Error: Path traversal attempt detected (directory name prefix). Access denied.\n");
        return NULL;
    }

    struct stat fileStat;
    if (stat(resolvedInput, &fileStat) != 0) {
        perror("Error getting file stats");
        return NULL;
    }
    if (!S_ISREG(fileStat.st_mode)) {
        fprintf(stderr, "Error: The specified path is not a regular file.\n");
        return NULL;
    }

    FILE* inFile = fopen(resolvedInput, "rb");
    if (!inFile) {
        perror("Error opening source file");
        return NULL;
    }

    char tempFileName[] = "/tmp/temp-copy-XXXXXX";
    int tempFd = mkstemp(tempFileName);
    if (tempFd == -1) {
        perror("Error creating temporary file");
        fclose(inFile);
        return NULL;
    }

    FILE* outFile = fdopen(tempFd, "wb");
    if (!outFile) {
        perror("Error opening temporary file stream");
        fclose(inFile);
        close(tempFd);
        unlink(tempFileName);
        return NULL;
    }
    
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    int errorOccurred = 0;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, inFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, outFile) != bytesRead) {
            fprintf(stderr, "Error writing to temporary file.\n");
            errorOccurred = 1;
            break;
        }
    }

    if (ferror(inFile)) {
        fprintf(stderr, "Error reading from source file.\n");
        errorOccurred = 1;
    }

    fclose(inFile);
    fclose(outFile);

    if (errorOccurred) {
        unlink(tempFileName);
        return NULL;
    }
    
    char* result = strdup(tempFileName);
    if (result == NULL) {
        perror("Error duplicating temp file name string");
        unlink(tempFileName);
        return NULL;
    }
    
    return result;
}

void setup_test_environment(const char* baseDir) {
    mkdir(baseDir, 0700);
    char filePath[PATH_MAX];
    snprintf(filePath, sizeof(filePath), "%s/valid.txt", baseDir);
    FILE* f = fopen(filePath, "w");
    if (f) {
        fputs("Hello, C!", f);
        fclose(f);
    }
    snprintf(filePath, sizeof(filePath), "%s/subdir", baseDir);
    mkdir(filePath, 0700);
}

void cleanup_test_environment(const char* baseDir) {
    char cmd[PATH_MAX + 10];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", baseDir);
    if(system(cmd) != 0) {
        fprintf(stderr, "Warning: Failed to cleanup test directory %s\n", baseDir);
    }
}

int main() {
    const char* testBaseDir = "/tmp/test-basedir-c";
    cleanup_test_environment(testBaseDir); // Clean before starting
    setup_test_environment(testBaseDir);

    printf("Test environment set up in: %s\n", testBaseDir);
    printf("------------------------------------------\n");

    printf("Test Case 1: Valid file 'valid.txt'\n");
    char* tempPath1 = copyFileToTemp(testBaseDir, "valid.txt");
    if (tempPath1) {
        printf("Success! Copied to: %s\n", tempPath1);
        unlink(tempPath1);
        free(tempPath1);
    } else {
        printf("Failed unexpectedly.\n");
    }
    printf("\n");

    printf("Test Case 2: Non-existent file 'nonexistent.txt'\n");
    char* tempPath2 = copyFileToTemp(testBaseDir, "nonexistent.txt");
    if (!tempPath2) {
        printf("Success! Operation failed as expected.\n");
    } else {
        printf("Failed! An unexpected temporary file was created: %s\n", tempPath2);
        unlink(tempPath2);
        free(tempPath2);
    }
    printf("\n");
    
    printf("Test Case 3: Path traversal attempt '../test.txt'\n");
    char* tempPath3 = copyFileToTemp(testBaseDir, "../test.txt");
    if (!tempPath3) {
        printf("Success! Path traversal blocked as expected.\n");
    } else {
        printf("Failed! Path traversal was successful: %s\n", tempPath3);
        unlink(tempPath3);
        free(tempPath3);
    }
    printf("\n");
    
    printf("Test Case 4: Accessing a directory 'subdir'\n");
    char* tempPath4 = copyFileToTemp(testBaseDir, "subdir");
    if (!tempPath4) {
        printf("Success! Directory access blocked as expected.\n");
    } else {
        printf("Failed! Directory was copied: %s\n", tempPath4);
        unlink(tempPath4);
        free(tempPath4);
    }
    printf("\n");

    printf("Test Case 5: Using a non-existent base directory\n");
    char nonExistentBaseDir[PATH_MAX];
    snprintf(nonExistentBaseDir, sizeof(nonExistentBaseDir), "%s/nonexistent_dir", testBaseDir);
    char* tempPath5 = copyFileToTemp(nonExistentBaseDir, "valid.txt");
    if (!tempPath5) {
        printf("Success! Invalid base directory handled correctly.\n");
    } else {
        printf("Failed! An unexpected file was created: %s\n", tempPath5);
        unlink(tempPath5);
        free(tempPath5);
    }

    cleanup_test_environment(testBaseDir);
    return 0;
}