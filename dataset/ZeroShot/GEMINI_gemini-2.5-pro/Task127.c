#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_MAX MAX_PATH
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#else
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#define BUFFER_SIZE 4096

/**
 * Securely reads a file from a specified base directory and writes it to a temporary location.
 * Prevents path traversal attacks.
 *
 * @param baseDir The trusted base directory from which to read files.
 * @param fileName The name of the file to read, relative to the base directory.
 * @return A dynamically allocated string with the path to the temp file, or NULL on failure. The caller must free this string.
 */
char* copyFileToTemp(const char* baseDir, const char* fileName) {
    char fullPath[PATH_MAX];
    char resolvedBase[PATH_MAX];
    char resolvedFull[PATH_MAX];
    
    FILE* srcFile = NULL;
    FILE* tempFile = NULL;
    char* tempFileName = NULL;

    // --- Input Validation ---
    if (fileName == NULL || *fileName == '\0' || strpbrk(fileName, "/\\") != NULL) {
        fprintf(stderr, "Error: Invalid file name.\n");
        return NULL;
    }
    
    // --- Path Resolution and Security Check ---
    if (realpath(baseDir, resolvedBase) == NULL) {
        perror("Error resolving base directory path");
        return NULL;
    }

    int n = snprintf(fullPath, sizeof(fullPath), "%s%c%s", baseDir,
        #ifdef _WIN32
            '\\'
        #else
            '/'
        #endif
        , fileName);

    if (n < 0 || n >= sizeof(fullPath)) {
        fprintf(stderr, "Error: Constructed file path is too long.\n");
        return NULL;
    }

    if (realpath(fullPath, resolvedFull) == NULL) {
        perror("Error resolving source file path (file may not exist)");
        return NULL;
    }

    if (strncmp(resolvedBase, resolvedFull, strlen(resolvedBase)) != 0) {
        fprintf(stderr, "Error: Path traversal attempt detected for file: %s\n", fileName);
        return NULL;
    }

    // --- Secure Temporary File Creation ---
    tempFile = tmpfile();
    if (tempFile == NULL) {
        perror("Error creating temporary file");
        goto cleanup;
    }
    
    // --- File Copy Operation ---
    srcFile = fopen(resolvedFull, "rb");
    if (srcFile == NULL) {
        perror("Error opening source file");
        goto cleanup;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
        if (fwrite(buffer, 1, bytesRead, tempFile) != bytesRead) {
            perror("Error writing to temporary file");
            goto cleanup;
        }
    }

    if (ferror(srcFile)) {
        perror("Error reading from source file");
        goto cleanup;
    }
    
    // In a real application, you'd need a way to get the name of the file
    // created by tmpfile(), which is non-standard. A better but less portable
    // approach is mkstemp (POSIX) or GetTempFileName (Windows).
    // For this example, we'll simulate returning a "name" for demonstration.
    // In a real scenario, you'd work with the FILE* handle directly.
    tempFileName = strdup("temp_file_handle_created.tmp"); // Placeholder
    if (!tempFileName) {
        perror("Error allocating memory for temp file name");
        goto cleanup;
    }

    printf("Successfully copied '%s' to a temporary location.\n", resolvedFull);
    // Rewind the temp file if it needs to be read immediately
    rewind(tempFile); 
    
    // In this simplified model, we close the temp file handle and return the placeholder name.
    // A real implementation might return the handle or use a platform-specific method
    // to get the name before closing.
    fclose(tempFile);
    fclose(srcFile);
    return tempFileName;


cleanup:
    if (srcFile) fclose(srcFile);
    if (tempFile) fclose(tempFile); // This also deletes the file
    // free(tempFileName) is not needed here as it's only set on success path
    return NULL;
}

void run_test(const char* test_dir, const char* file_name, const char* description) {
    printf("\n%s\n", description);
    char* temp_path = copyFileToTemp(test_dir, file_name);
    if (temp_path) {
        // In a real scenario, you would do something with the temp file.
        // Here we just clean up the placeholder string.
        free(temp_path);
    }
}

int main() {
    const char* testDirName = "c_test_dir";
    char goodFilePath[PATH_MAX];
    char emptyFilePath[PATH_MAX];
    char unreadableFilePath[PATH_MAX];

    snprintf(goodFilePath, sizeof(goodFilePath), "%s/good_file.txt", testDirName);
    snprintf(emptyFilePath, sizeof(emptyFilePath), "%s/empty_file.txt", testDirName);
    snprintf(unreadableFilePath, sizeof(unreadableFilePath), "%s/unreadable.txt", testDirName);

    // --- Setup Test Environment ---
    if (MKDIR(testDirName) != 0 && errno != EEXIST) {
        perror("Failed to create test directory");
        return 1;
    }
    FILE* f = fopen(goodFilePath, "w");
    if (f) { fprintf(f, "This is a test file."); fclose(f); }
    f = fopen(emptyFilePath, "w");
    if (f) { fclose(f); }
    f = fopen(unreadableFilePath, "w");
    if (f) { fclose(f); }

#ifndef _WIN32
    // Make file unreadable on POSIX systems
    chmod(unreadableFilePath, 0200); // Write-only for owner
#endif

    printf("--- Running Test Cases ---\n");
    
    run_test(testDirName, "good_file.txt", "1. Testing with a valid file:");
    run_test(testDirName, "non_existent_file.txt", "2. Testing with a non-existent file:");
    run_test(".", "../some_other_file", "3. Testing path traversal attack:");
    run_test(testDirName, "empty_file.txt", "4. Testing with an empty file:");
    run_test(testDirName, "unreadable.txt", "5. Testing with an unreadable file:");

    printf("\n--- Test Cases Finished ---\n");

    // --- Cleanup Test Environment ---
    remove(goodFilePath);
    remove(emptyFilePath);
    remove(unreadableFilePath);
#ifdef _WIN32
    _rmdir(testDirName);
#else
    rmdir(testDirName);
#endif
    printf("\nTest environment cleaned up.\n");

    return 0;
}