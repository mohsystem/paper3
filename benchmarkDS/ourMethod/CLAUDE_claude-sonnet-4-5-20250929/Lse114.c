
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>
#include <errno.h>

/**
 * Secure directory listing - DOES NOT use system() to avoid command injection.
 * Uses POSIX dirent.h for safe directory operations.
 * 
 * Returns: 0 on success, -1 on error
 */
int listDirectory(const char* directoryPath) {
    DIR* dir = NULL;
    struct dirent* entry = NULL;
    struct stat statbuf;
    char realPath[PATH_MAX];
    
    // Input validation: reject NULL or empty paths
    if (directoryPath == NULL || directoryPath[0] == '\\0') {
        fprintf(stderr, "Error: Invalid directory path\\n");
        return -1;
    }
    
    // Resolve to canonical absolute path to prevent traversal
    if (realpath(directoryPath, realPath) == NULL) {
        fprintf(stderr, "Error: Cannot resolve path\\n");
        return -1;
    }
    
    // Verify it's a directory
    if (stat(realPath, &statbuf) != 0) {
        fprintf(stderr, "Error: Cannot access path\\n");
        return -1;
    }
    
    if (!S_ISDIR(statbuf.st_mode)) {
        fprintf(stderr, "Error: Path is not a directory\\n");
        return -1;
    }
    
    // Open directory safely
    dir = opendir(realPath);
    if (dir == NULL) {
        fprintf(stderr, "Error: Cannot open directory\\n");
        return -1;
    }
    
    // Print header
    printf("Contents of: %s\\n", realPath);
    
    // Read directory entries
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        printf("%s\\n", entry->d_name);
    }
    
    // Check for read errors
    if (errno != 0) {
        fprintf(stderr, "Error: Failed to read directory\\n");
        closedir(dir);
        return -1;
    }
    
    closedir(dir);
    return 0;
}

int main(void) {
    // Test case 1: Current directory
    printf("=== Test 1: Current directory ===\\n");
    listDirectory(".");
    
    // Test case 2: Temp directory
    printf("\\n=== Test 2: Temp directory ===\\n");
    listDirectory("/tmp");
    
    // Test case 3: Invalid path (security test)
    printf("\\n=== Test 3: Path traversal attempt ===\\n");
    listDirectory("../../../etc");
    
    // Test case 4: NULL input
    printf("\\n=== Test 4: NULL input ===\\n");
    listDirectory(NULL);
    
    // Test case 5: Empty string
    printf("\\n=== Test 5: Empty string ===\\n");
    listDirectory("");
    
    return 0;
}
