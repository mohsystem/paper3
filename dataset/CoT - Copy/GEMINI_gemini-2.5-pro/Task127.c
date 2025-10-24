#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#define SEPARATOR "\\"
#else
#include <unistd.h>
#include <sys/stat.h>
#define SEPARATOR "/"
#endif

#define BUFFER_SIZE 1024

/**
 * Reads a file from sourcePath and writes its content to destPath.
 * Handles errors by checking return values and errno.
 *
 * @param sourcePath The path of the file to read.
 * @param destPath The path of the file to write.
 * @return 0 on success, -1 on failure (errno is set).
 */
int copyFileWithHandling(const char* sourcePath, const char* destPath) {
    FILE *sourceFile = NULL;
    FILE *destFile = NULL;
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    int ret_val = -1; // Default to failure

    // 1. Open source file for reading
    sourceFile = fopen(sourcePath, "rb");
    if (sourceFile == NULL) {
        // perror provides a descriptive error message based on errno
        fprintf(stderr, "Error: Cannot open source file '%s'. ", sourcePath);
        perror(NULL);
        goto cleanup;
    }

    // 2. Open destination file for writing
    destFile = fopen(destPath, "wb");
    if (destFile == NULL) {
        fprintf(stderr, "Error: Cannot open destination file '%s'. ", destPath);
        perror(NULL);
        goto cleanup;
    }

    // 3. Read from source and write to destination
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, sourceFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, destFile) != bytesRead) {
            fprintf(stderr, "Error: Failed to write to destination file '%s'. ", destPath);
            perror(NULL);
            goto cleanup;
        }
    }
    
    // 4. Check for read errors
    if (ferror(sourceFile)) {
        fprintf(stderr, "Error: Failed to read from source file '%s'. ", sourcePath);
        perror(NULL);
        goto cleanup;
    }

    printf("Successfully copied '%s' to '%s'.\n", sourcePath, destPath);
    ret_val = 0; // Success

cleanup:
    // 5. Close files if they were opened
    if (sourceFile != NULL) {
        if (fclose(sourceFile) != 0) {
            fprintf(stderr, "Warning: Failed to close source file '%s'. ", sourcePath);
            perror(NULL);
            ret_val = -1; // Mark as failure if not already
        }
    }
    if (destFile != NULL) {
        if (fclose(destFile) != 0) {
            fprintf(stderr, "Warning: Failed to close destination file '%s'. ", destPath);
            perror(NULL);
            ret_val = -1; // Mark as failure if not already
        }
    }

    return ret_val;
}

int main() {
    // Setup for test cases
    const char* tempDir = getenv("TMP") ? getenv("TMP") : (getenv("TEMP") ? getenv("TEMP") : "/tmp");
    char sourceFilePath[256];
    char destFilePath[256];
    char nonExistentSource[256];
    char writeProtectedDir[256];

    snprintf(sourceFilePath, sizeof(sourceFilePath), "%s%ssource.txt", tempDir, SEPARATOR);
    snprintf(destFilePath, sizeof(destFilePath), "%s%sdestination.txt", tempDir, SEPARATOR);
    snprintf(nonExistentSource, sizeof(nonExistentSource), "%s%snon_existent_file.txt", tempDir, SEPARATOR);
    snprintf(writeProtectedDir, sizeof(writeProtectedDir), "%s%sprotected_dir", tempDir, SEPARATOR);


    // Create a dummy source file
    FILE* f = fopen(sourceFilePath, "w");
    if (f) {
        fputs("This is a test file.", f);
        fclose(f);
    } else {
        perror("Failed to set up test cases");
        return 1;
    }
    
    // Create a directory to simulate a write-protected location
    #ifdef _WIN32
        CreateDirectory(writeProtectedDir, NULL);
    #else
        mkdir(writeProtectedDir, 0755);
    #endif


    printf("--- Running C Test Cases ---\n");

    // Test Case 1: Successful file copy
    printf("\n[Test Case 1: Successful Copy]\n");
    copyFileWithHandling(sourceFilePath, destFilePath);

    // Test Case 2: Source file does not exist
    printf("\n[Test Case 2: Source File Not Found]\n");
    copyFileWithHandling(nonExistentSource, destFilePath);

    // Test Case 3: Reading a directory (simulates a read error)
    printf("\n[Test Case 3: Read Error (source is a directory)]\n");
    copyFileWithHandling(writeProtectedDir, destFilePath);

    // Test Case 4: Destination is not writable (it's a directory)
    printf("\n[Test Case 4: Write Error (destination is a directory)]\n");
    copyFileWithHandling(sourceFilePath, writeProtectedDir);

    // Test Case 5: Invalid path (NULL)
    printf("\n[Test Case 5: Invalid Argument (null path)]\n");
    copyFileWithHandling(NULL, destFilePath); // fopen will set errno to EINVAL or EFAULT

    // Cleanup
    remove(sourceFilePath);
    remove(destFilePath);
    #ifdef _WIN32
        RemoveDirectory(writeProtectedDir);
    #else
        rmdir(writeProtectedDir);
    #endif

    return 0;
}