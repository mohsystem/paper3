
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdbool.h>

/* Validates that the path is within allowed base directory and is a regular file */
/* Returns true if valid, false otherwise */
bool validateFilePath(const char* basePath, const char* userPath, char* resolvedPath, size_t resolvedSize) {
    /* Input validation: check for null pointers */
    if (basePath == NULL || userPath == NULL || resolvedPath == NULL) {
        fprintf(stderr, "Error: Null pointer provided to validateFilePath\\n");
        return false;
    }
    
    /* Initialize output buffer */
    memset(resolvedPath, 0, resolvedSize);
    
    /* Validate resolvedSize is sufficient */
    if (resolvedSize < PATH_MAX) {
        fprintf(stderr, "Error: Resolved path buffer too small\\n");
        return false;
    }
    
    /* Validate input length to prevent buffer overflow */
    size_t userPathLen = strlen(userPath);
    if (userPathLen > PATH_MAX) {
        fprintf(stderr, "Error: Path too long\\n");
        return false;
    }
    
    /* Check for empty path */
    if (userPathLen == 0) {
        fprintf(stderr, "Error: Empty path provided\\n");
        return false;
    }
    
    /* Check for dangerous path characters and sequences */
    if (strstr(userPath, "..") != NULL) {
        fprintf(stderr, "Error: Path traversal detected (..)\\n");
        return false;
    }
    
    /* Resolve base path to canonical form */
    char resolvedBase[PATH_MAX];
    memset(resolvedBase, 0, sizeof(resolvedBase));
    if (realpath(basePath, resolvedBase) == NULL) {
        fprintf(stderr, "Error: Cannot resolve base directory\\n");
        return false;
    }
    
    /* Resolve user path to canonical form */
    if (realpath(userPath, resolvedPath) == NULL) {
        fprintf(stderr, "Error: Cannot resolve user path or file does not exist\\n");
        return false;
    }
    
    /* Ensure resolved path starts with base directory */
    size_t baseLen = strlen(resolvedBase);
    if (strncmp(resolvedPath, resolvedBase, baseLen) != 0) {
        fprintf(stderr, "Error: Path escapes base directory\\n");
        return false;
    }
    
    /* Ensure there's a path separator after base (unless path equals base) */\n    if (strlen(resolvedPath) > baseLen && resolvedPath[baseLen] != '/') {\n        fprintf(stderr, "Error: Invalid path structure\\n");\n        return false;\n    }\n    \n    /* Check if path is a regular file (not directory or symlink) */\n    struct stat statbuf;\n    memset(&statbuf, 0, sizeof(statbuf));\n    if (lstat(resolvedPath, &statbuf) != 0) {\n        fprintf(stderr, "Error: Cannot stat file\\n");\n        return false;\n    }\n    \n    /* Reject symlinks to prevent TOCTOU attacks */\n    if (S_ISLNK(statbuf.st_mode)) {\n        fprintf(stderr, "Error: Symlinks are not allowed\\n");\n        return false;\n    }\n    \n    /* Ensure it's a regular file */
    if (!S_ISREG(statbuf.st_mode)) {
        fprintf(stderr, "Error: Not a regular file\\n");
        return false;
    }
    
    return true;
}

/* Safely deletes a file with validation */
bool deleteFile(const char* basePath, const char* filePath) {
    char resolvedPath[PATH_MAX];
    memset(resolvedPath, 0, sizeof(resolvedPath));
    
    /* Validate the file path before deletion */
    if (!validateFilePath(basePath, filePath, resolvedPath, sizeof(resolvedPath))) {
        return false;
    }
    
    /* Perform atomic deletion using unlink */
    /* Using unlink instead of remove to be explicit about deleting files only */
    if (unlink(resolvedPath) != 0) {
        fprintf(stderr, "Error: Failed to delete file: %s\\n", strerror(errno));
        return false;
    }
    
    printf("File deleted successfully: %s\\n", resolvedPath);
    return true;
}

int main(int argc, char* argv[]) {
    /* Initialize pointers */
    char* baseDir = NULL;
    char baseDirBuffer[PATH_MAX];
    memset(baseDirBuffer, 0, sizeof(baseDirBuffer));
    
    /* Validate command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\\n", argv[0] != NULL ? argv[0] : "program");
        return 1;
    }
    
    /* Validate argv[1] is not NULL */
    if (argv[1] == NULL) {
        fprintf(stderr, "Error: File path argument is NULL\\n");
        return 1;
    }
    
    /* Get base directory (current working directory for this example) */
    baseDir = getcwd(baseDirBuffer, sizeof(baseDirBuffer));
    if (baseDir == NULL) {
        fprintf(stderr, "Error: Cannot get current directory\\n");
        return 1;
    }
    
    /* Delete the specified file */
    if (!deleteFile(baseDir, argv[1])) {
        return 1;
    }
    
    /* Test cases (commented out for production use) */
    /* Uncomment to run tests - these would need test files created first */
    /*
    printf("\\n=== Running Test Cases ===\\n");
    
    // Test 1: Valid file in current directory
    printf("\\nTest 1: Delete valid file\\n");
    FILE* f1 = fopen("test1.txt", "w");
    if (f1 != NULL) { 
        fprintf(f1, "test"); 
        fclose(f1); 
        f1 = NULL;
    }
    deleteFile(baseDir, "test1.txt");
    
    // Test 2: Attempt path traversal
    printf("\\nTest 2: Path traversal attempt (should fail)\\n");
    deleteFile(baseDir, "../etc/passwd");
    
    // Test 3: Non-existent file
    printf("\\nTest 3: Non-existent file (should fail)\\n");
    deleteFile(baseDir, "nonexistent.txt");
    
    // Test 4: Attempt to delete directory
    printf("\\nTest 4: Attempt to delete directory (should fail)\\n");
    deleteFile(baseDir, ".");
    
    // Test 5: Valid file with absolute path
    printf("\\nTest 5: Valid file with absolute path\\n");
    char testPath[PATH_MAX];
    memset(testPath, 0, sizeof(testPath));
    int written = snprintf(testPath, sizeof(testPath), "%s/test5.txt", baseDir);
    if (written > 0 && (size_t)written < sizeof(testPath)) {
        FILE* f5 = fopen(testPath, "w");
        if (f5 != NULL) { 
            fprintf(f5, "test"); 
            fclose(f5); 
            f5 = NULL;
        }
        deleteFile(baseDir, testPath);
    }
    */
    
    return 0;
}
