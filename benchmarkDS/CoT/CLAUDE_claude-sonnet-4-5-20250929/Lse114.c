
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

// SECURE ALTERNATIVE: Use POSIX directory functions
void listDirectorySecure(const char* directoryPath) {
    DIR *dir;
    struct dirent *entry;
    struct stat statbuf;
    
    // Validate input
    if (directoryPath == NULL || strlen(directoryPath) == 0) {
        printf("Invalid directory path\\n");
        return;
    }
    
    // Check if directory exists
    if (stat(directoryPath, &statbuf) != 0) {
        printf("Error accessing path: %s\\n", strerror(errno));
        return;
    }
    
    if (!S_ISDIR(statbuf.st_mode)) {
        printf("Path is not a directory: %s\\n", directoryPath);
        return;
    }
    
    // Open directory securely
    dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("Error opening directory: %s\\n", strerror(errno));
        return;
    }
    
    // List directory contents
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

// WARNING: INSECURE - For educational purposes only
// DO NOT USE IN PRODUCTION
void listDirectoryUnsafe(const char* directory) {
    printf("WARNING: system() calls are extremely insecure\\n");
    printf("They are vulnerable to command injection attacks\\n");
    printf("NEVER use system() with user input in production code\\n");
    // Intentionally not implementing: system() call
}

int main() {
    printf("=== Secure Directory Listing ===\\n\\n");
    
    // Test case 1: Current directory
    printf("Test 1 - Current directory:\\n");
    listDirectorySecure(".");
    
    // Test case 2: Parent directory
    printf("\\nTest 2 - Parent directory:\\n");
    listDirectorySecure("..");
    
    // Test case 3: Temp directory
    printf("\\nTest 3 - Temp directory:\\n");
    listDirectorySecure("/tmp");
    
    // Test case 4: Invalid path
    printf("\\nTest 4 - Invalid path:\\n");
    listDirectorySecure("/nonexistent/path");
    
    // Test case 5: Root directory
    printf("\\nTest 5 - Root directory:\\n");
    listDirectorySecure("/");
    
    return 0;
}
