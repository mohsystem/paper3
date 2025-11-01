#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>

// A predefined base directory to prevent access to other parts of the filesystem.
const char* IMAGE_BASE_DIRECTORY = "test_images";

// Define a struct to hold the result
typedef struct {
    char* data;
    long size;
    bool success;
} FileData;

/**
 * Simulates fetching an image file based on a 'file' parameter.
 * This function securely reads a file from a predefined base directory,
 * preventing path traversal attacks.
 *
 * NOTE: The caller is responsible for freeing the `data` member of the returned
 *       struct if the call was successful.
 *
 * @param fileName The name of the file requested. Must not contain path separators.
 * @return A FileData struct. If successful, `success` is true, `data` points
 *         to the file content, and `size` is the file size. Otherwise, `success`
 *         is false, and `data` is NULL.
 */
FileData img(const char* fileName) {
    FileData result = {NULL, 0, false};

    if (fileName == NULL || *fileName == '\0') {
        fprintf(stderr, "Error: File name is null or empty.\n");
        return result;
    }

    // Security Check: Disallow path separators and ".." to prevent traversal.
    // A more robust solution would use realpath() on POSIX systems, but this is more portable.
    if (strstr(fileName, "..") != NULL || strchr(fileName, '/') != NULL || strchr(fileName, '\\') != NULL) {
        fprintf(stderr, "Security Error: Invalid characters in file name: %s\n", fileName);
        return result;
    }

    // Construct the full path securely. 1 for '/', 1 for null terminator.
    char* fullPath = malloc(strlen(IMAGE_BASE_DIRECTORY) + strlen(fileName) + 2);
    if (fullPath == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return result;
    }
    sprintf(fullPath, "%s/%s", IMAGE_BASE_DIRECTORY, fileName);

    FILE* file = fopen(fullPath, "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: Cannot open file: %s\n", fullPath);
        free(fullPath);
        return result;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize < 0) {
        fprintf(stderr, "Error: Could not determine file size for: %s\n", fullPath);
        fclose(file);
        free(fullPath);
        return result;
    }
    
    // Allocate memory for the file content
    char* buffer = (char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for file content.\n");
        fclose(file);
        free(fullPath);
        return result;
    }

    // Read the file into the buffer
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != (size_t)fileSize) {
        fprintf(stderr, "Error: Failed to read the entire file: %s\n", fullPath);
        free(buffer);
        fclose(file);
        free(fullPath);
        return result;
    }

    fclose(file);
    free(fullPath);

    result.data = buffer;
    result.size = fileSize;
    result.success = true;
    return result;
}

void setup_test_environment() {
    #ifdef _WIN32
        _mkdir(IMAGE_BASE_DIRECTORY);
    #else
        mkdir(IMAGE_BASE_DIRECTORY, 0755);
    #endif

    char safe_path[256];
    sprintf(safe_path, "%s/test1.jpg", IMAGE_BASE_DIRECTORY);
    
    FILE *safeFile = fopen(safe_path, "wb");
    if (safeFile) {
        fputs("ImageData1", safeFile);
        fclose(safeFile);
    }

    FILE *secretFile = fopen("secret.txt", "wb");
    if (secretFile) {
        fputs("SecretData", secretFile);
        fclose(secretFile);
    }
    printf("Test environment set up.\n");
}

void cleanup_test_environment() {
    char safe_path[256];
    sprintf(safe_path, "%s/test1.jpg", IMAGE_BASE_DIRECTORY);
    remove(safe_path);
    remove(IMAGE_BASE_DIRECTORY);
    remove("secret.txt");
    printf("\n--- Test environment cleaned up. ---\n");
}

int main() {
    setup_test_environment();
    
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Valid file
    printf("Test 1: Requesting a valid file 'test1.jpg'\n");
    FileData r1 = img("test1.jpg");
    if (r1.success && r1.size == 10 && strncmp(r1.data, "ImageData1", 10) == 0) {
        printf("  Result: SUCCESS, content matches.\n");
    } else {
        printf("  Result: FAILED.\n");
    }
    free(r1.data);

    // Test Case 2: Non-existent file
    printf("\nTest 2: Requesting a non-existent file 'notfound.jpg'\n");
    FileData r2 = img("notfound.jpg");
    if (!r2.success) {
        printf("  Result: SUCCESS, function failed as expected.\n");
    } else {
        printf("  Result: FAILED.\n");
    }
    free(r2.data); // free(NULL) is safe

    // Test Case 3: Path traversal attempt with '..'
    printf("\nTest 3: Path traversal attempt '../secret.txt'\n");
    FileData r3 = img("../secret.txt");
    if (!r3.success) {
        printf("  Result: SUCCESS, path traversal blocked.\n");
    } else {
        printf("  Result: FAILED.\n");
    }
    free(r3.data);

    // Test Case 4: Path traversal attempt with '/'
    printf("\nTest 4: Path traversal attempt with absolute path '/etc/passwd'\n");
    FileData r4 = img("/etc/passwd");
    if (!r4.success) {
        printf("  Result: SUCCESS, absolute path blocked.\n");
    } else {
        printf("  Result: FAILED.\n");
    }
    free(r4.data);

    // Test Case 5: Path traversal attempt with '\'
    printf("\nTest 5: Path traversal attempt '..\\secret.txt'\n");
    FileData r5 = img("..\\secret.txt");
    if (!r5.success) {
        printf("  Result: SUCCESS, path traversal blocked.\n");
    } else {
        printf("  Result: FAILED.\n");
    }
    free(r5.data);
    
    cleanup_test_environment();
    return 0;
}