#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define PATH_MAX MAX_PATH
#define realpath(N, R) _fullpath((R), (N), PATH_MAX)
#define MKDIR(path) _mkdir(path)
#define SEP "\\"
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#define SEP "/"
#endif

typedef struct {
    char* data;
    long size;
} FileContent;

/**
 * Securely reads a file from a base directory, preventing path traversal.
 *
 * @param baseDirectory The secure directory from which files can be served.
 * @param fileName The name of the file requested by the user.
 * @return A FileContent struct. If successful, 'data' points to a heap-allocated
 *         buffer with the file content and 'size' is the file size. 'data' MUST
 *         be freed by the caller. On failure, 'data' is NULL and 'size' is 0.
 */
FileContent img(const char* baseDirectory, const char* fileName) {
    FileContent result = {NULL, 0};
    char fullPath[PATH_MAX];
    char resolvedBase[PATH_MAX];
    char resolvedRequest[PATH_MAX];

    // Construct the full path safely
    int written = snprintf(fullPath, sizeof(fullPath), "%s%s%s", baseDirectory, SEP, fileName);
    if (written < 0 || written >= sizeof(fullPath)) {
        fprintf(stderr, "Error: Constructed path is too long.\n");
        return result;
    }

    // *** SECURITY CHECK ***
    // To prevent path traversal, resolve the canonical paths of both the base
    // directory and the requested file. Then, verify that the requested file's
    // path starts with the base directory's path.

    // 1. Resolve the base directory's absolute path.
    if (realpath(baseDirectory, resolvedBase) == NULL) {
        // Base directory does not exist or is not accessible.
        // perror("realpath (base)");
        return result;
    }

    // 2. Resolve the requested file's absolute path.
    if (realpath(fullPath, resolvedRequest) == NULL) {
        // Requested file does not exist or is not accessible.
        // perror("realpath (request)");
        return result;
    }
    
    // 3. Compare the two paths.
    size_t baseLen = strlen(resolvedBase);
    if (strncmp(resolvedRequest, resolvedBase, baseLen) != 0) {
        fprintf(stderr, "Security alert: Path traversal attempt detected for file: %s\n", fileName);
        return result;
    }
    
    // 4. Ensure it's a real subdirectory, not just a matching prefix
    // (e.g., /base/dir vs /base/directory).
    if (resolvedRequest[baseLen] != '\0' && resolvedRequest[baseLen] != SEP[0]) {
        fprintf(stderr, "Security alert: Path prefix confusion attempt for file: %s\n", fileName);
        return result;
    }
    
    // Open the file for reading in binary mode
    FILE *file = fopen(resolvedRequest, "rb");
    if (file == NULL) {
        // perror("fopen");
        return result;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if (fileSize < 0) {
        fclose(file);
        return result;
    }
    fseek(file, 0, SEEK_SET);

    // Allocate memory and read file content
    char* buffer = (char*)malloc(fileSize);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Could not allocate memory for file.\n");
        fclose(file);
        return result;
    }

    if (fread(buffer, 1, fileSize, file) != fileSize) {
        fprintf(stderr, "Error: Failed to read the entire file.\n");
        free(buffer);
        fclose(file);
        return result;
    }
    
    fclose(file);
    result.data = buffer;
    result.size = fileSize;
    return result;
}

void runTest(const char* testName, const char* baseDir, const char* fileName) {
    printf("--- Running Test: %s ---\n", testName);
    printf("Requesting file: %s\n", fileName);
    FileContent content = img(baseDir, fileName);
    if (content.data != NULL) {
        printf("Result: SUCCESS - Read %ld bytes.\n", content.size);
        free(content.data); // IMPORTANT: Free the allocated memory
    } else {
        printf("Result: FAILED (or file securely blocked) - as expected for invalid cases.\n");
    }
    printf("\n");
}

// Simple helper to create a file with content
void createFile(const char* path, const char* content) {
    FILE* f = fopen(path, "w");
    if (f) {
        fputs(content, f);
        fclose(f);
    }
}

int main() {
    const char* baseDir = "safe_image_dir";
    const char* unsafeDir = "unsafe_dir";

    // Setup
    MKDIR(baseDir);
    char subdirPath[256];
    snprintf(subdirPath, sizeof(subdirPath), "%s%ssubdir", baseDir, SEP);
    MKDIR(subdirPath);
    MKDIR(unsafeDir);

    char path1[256], path2[256], path3[256];
    snprintf(path1, sizeof(path1), "%s%simage1.jpg", baseDir, SEP);
    snprintf(path2, sizeof(path2), "%s%simage2.png", subdirPath, SEP);
    snprintf(path3, sizeof(path3), "%s%ssecret.txt", unsafeDir, SEP);
    createFile(path1, "jpeg_data_for_image1");
    createFile(path2, "png_data_for_image2");
    createFile(path3, "this is a secret file");

    // --- Test Cases ---
    // 1. Valid file in base directory
    runTest("Valid File", baseDir, "image1.jpg");

    // 2. Valid file in subdirectory
    char subdirFile[256];
    snprintf(subdirFile, sizeof(subdirFile), "subdir%simage2.png", SEP);
    runTest("Valid Subdirectory File", baseDir, subdirFile);

    // 3. Non-existent file
    runTest("Non-Existent File", baseDir, "notfound.gif");

    // 4. Path traversal attack
    char traversalPath[256];
    snprintf(traversalPath, sizeof(traversalPath), ".." SEP "%s" SEP "secret.txt", unsafeDir);
    runTest("Path Traversal Attack", baseDir, traversalPath);
    
    // 5. Absolute path attack
    char absolutePath[PATH_MAX];
    if (realpath(path3, absolutePath)) {
        runTest("Absolute Path Attack", baseDir, absolutePath);
    } else {
        printf("Could not resolve absolute path for test case 5.\n");
    }
    
    // Cleanup
    remove(path1);
    remove(path2);
    remove(path3);
#ifdef _WIN32
    _rmdir(subdirPath);
    _rmdir(baseDir);
    _rmdir(unsafeDir);
#else
    rmdir(subdirPath);
    rmdir(baseDir);
    rmdir(unsafeDir);
#endif

    return 0;
}