#define _GNU_SOURCE // For realpath on some systems
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#define PATH_MAX MAX_PATH
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

#define SAFE_DIRECTORY_NAME "safe_dir"

/**
 * @brief Securely reads a file from a predefined safe directory.
 *
 * This function prevents path traversal attacks by canonicalizing paths using
 * realpath (on POSIX) or GetFullPathName (on Windows) and verifying the requested 
 * file is strictly within the safe directory.
 * 
 * @param filename The name of the file to read.
 * @param file_content A pointer to a char pointer that will be allocated and
 *                     filled with the file's content. The caller is responsible
 *                     for freeing this memory.
 * @return The size of the file in bytes on success, or -1 on failure.
 */
long readFileFromSafeDirectory(const char* filename, char** file_content) {
    if (filename == NULL || *filename == '\0' || strchr(filename, '\0') != filename + strlen(filename)) {
        fprintf(stderr, "Error: Filename is invalid (NULL, empty, or contains embedded null bytes).\n");
        return -1;
    }
    
    char safe_dir_path[PATH_MAX];
    char target_path[PATH_MAX];
    char safe_dir_canon[PATH_MAX];
    char target_canon[PATH_MAX];

    // Get canonical path of the safe directory
#ifdef _WIN32
    if (GetFullPathName(SAFE_DIRECTORY_NAME, PATH_MAX, safe_dir_canon, NULL) == 0) {
        fprintf(stderr, "Error: Could not resolve path for safe directory.\n");
        return -1;
    }
#else
    if (realpath(SAFE_DIRECTORY_NAME, safe_dir_canon) == NULL) {
        fprintf(stderr, "Error: Could not resolve path for safe directory: %s.\n", strerror(errno));
        return -1;
    }
#endif

    // Construct full path to target file
    snprintf(target_path, sizeof(target_path), "%s/%s", SAFE_DIRECTORY_NAME, filename);
    
    // Get canonical path of the target file
#ifdef _WIN32
    if (GetFullPathName(target_path, PATH_MAX, target_canon, NULL) == 0) {
        fprintf(stderr, "Error: File not found or path is invalid: %s\n", filename);
        return -1;
    }
#else
    if (realpath(target_path, target_canon) == NULL) {
        fprintf(stderr, "Error: File not found or path is invalid: %s. %s\n", filename, strerror(errno));
        return -1;
    }
#endif

    // The crucial security check: is the canonical target path within the canonical safe directory?
    size_t safe_dir_len = strlen(safe_dir_canon);
    if (strncmp(target_canon, safe_dir_canon, safe_dir_len) != 0 ||
        (target_canon[safe_dir_len] != '/' && target_canon[safe_dir_len] != '\\' && target_canon[safe_dir_len] != '\0')) {
        fprintf(stderr, "Security Error: Path traversal attempt detected for file: %s\n", filename);
        return -1;
    }
    
    // Final check to ensure we're reading a regular file
    struct stat statbuf;
    if (stat(target_canon, &statbuf) != 0 || !S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file: %s\n", filename);
        return -1;
    }

    FILE* fp = fopen(target_canon, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Could not open file: %s. %s\n", filename, strerror(errno));
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size < 0) {
        fclose(fp);
        fprintf(stderr, "Error: Could not determine file size.\n");
        return -1;
    }

    *file_content = (char*)malloc(size + 1);
    if (!*file_content) {
        fclose(fp);
        fprintf(stderr, "Error: Could not allocate memory for file content.\n");
        return -1;
    }

    if (fread(*file_content, 1, size, fp) != (size_t)size) {
        fclose(fp);
        free(*file_content);
        *file_content = NULL;
        fprintf(stderr, "Error: Could not read entire file.\n");
        return -1;
    }
    
    (*file_content)[size] = '\0';
    fclose(fp);
    return size;
}

void setupTestEnvironment() {
    printf("--- Setting up test environment ---\n");
    MKDIR(SAFE_DIRECTORY_NAME);
    char subdir_path[256];
    snprintf(subdir_path, sizeof(subdir_path), "%s/subdir", SAFE_DIRECTORY_NAME);
    MKDIR(subdir_path);

    FILE* fp1 = fopen(SAFE_DIRECTORY_NAME "/test1.txt", "w");
    if (fp1) {
        fprintf(fp1, "This is a safe file.");
        fclose(fp1);
    }
    FILE* fp2 = fopen(SAFE_DIRECTORY_NAME "/subdir/test2.txt", "w");
     if (fp2) {
        fprintf(fp2, "This is another safe file in a subdirectory.");
        fclose(fp2);
    }
    FILE* fp3 = fopen("secret.txt", "w");
    if (fp3) {
        fprintf(fp3, "This is a secret file.");
        fclose(fp3);
    }
    printf("Setup complete.\n");
}

void cleanupTestEnvironment() {
    printf("\n--- Cleaning up test environment ---\n");
    remove(SAFE_DIRECTORY_NAME "/subdir/test2.txt");
    remove(SAFE_DIRECTORY_NAME "/test1.txt");
#ifdef _WIN32
    _rmdir(SAFE_DIRECTORY_NAME "/subdir");
    _rmdir(SAFE_DIRECTORY_NAME);
#else
    rmdir(SAFE_DIRECTORY_NAME "/subdir");
    rmdir(SAFE_DIRECTORY_NAME);
#endif
    remove("secret.txt");
    printf("Cleanup complete.\n");
}

int main() {
    setupTestEnvironment();
    printf("\n--- Running Test Cases ---\n");
    char* content = NULL;
    long size;

    // Test Case 1: Read a valid file
    printf("\n[Test 1] Reading a valid file 'test1.txt'\n");
    size = readFileFromSafeDirectory("test1.txt", &content);
    if (size != -1) {
        printf("SUCCESS. Content: %s\n", content);
        free(content);
        content = NULL;
    } else {
        printf("FAILED.\n");
    }

    // Test Case 2: Read a non-existent file
    printf("\n[Test 2] Reading a non-existent file 'nonexistent.txt'\n");
    size = readFileFromSafeDirectory("nonexistent.txt", &content);
    if (size == -1) {
        printf("FAILED. (Expected)\n");
    }

    // Test Case 3: Path traversal attempt
    printf("\n[Test 3] Path traversal attempt '../secret.txt'\n");
    size = readFileFromSafeDirectory("../secret.txt", &content);
    if (size == -1) {
        printf("FAILED. (Expected Security Block)\n");
    }

    // Test Case 4: Path traversal from a subdirectory
    printf("\n[Test 4] Path traversal from subdirectory 'subdir/../../secret.txt'\n");
    size = readFileFromSafeDirectory("subdir/../../secret.txt", &content);
    if (size == -1) {
        printf("FAILED. (Expected Security Block)\n");
    }

    // Test Case 5: Reading a valid file in a subdirectory
    printf("\n[Test 5] Reading a valid file in a subdirectory 'subdir/test2.txt'\n");
    size = readFileFromSafeDirectory("subdir/test2.txt", &content);
    if (size != -1) {
        printf("SUCCESS. Content: %s\n", content);
        free(content);
        content = NULL;
    } else {
        printf("FAILED.\n");
    }

    cleanupTestEnvironment();
    return 0;
}