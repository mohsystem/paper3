
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>

// Secure method: Uses opendir/readdir instead of system()
void listDirectory(const char* directoryPath) {
    struct stat statbuf;
    
    if (directoryPath == NULL || strlen(directoryPath) == 0) {
        printf("Invalid directory path\\n");
        return;
    }
    
    // Check if path exists
    if (stat(directoryPath, &statbuf) != 0) {
        printf("Directory does not exist: %s\\n", directoryPath);
        return;
    }
    
    // Check if path is a directory
    if (!S_ISDIR(statbuf.st_mode)) {
        printf("Path is not a directory: %s\\n", directoryPath);
        return;
    }
    
    DIR* dir = opendir(directoryPath);
    if (dir == NULL) {
        printf("Error opening directory: %s\\n", strerror(errno));
        return;
    }
    
    printf("Contents of %s:\\n", directoryPath);
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and .. entries
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

int main() {
    printf("=== Test Case 1: Current directory ===\\n");
    listDirectory(".");
    
    printf("\\n=== Test Case 2: Home directory ===\\n");
    const char* home = getenv("HOME");
    if (home) listDirectory(home);
    
    printf("\\n=== Test Case 3: Temp directory ===\\n");
    listDirectory("/tmp");
    
    printf("\\n=== Test Case 4: Non-existent directory ===\\n");
    listDirectory("/nonexistent/path");
    
    printf("\\n=== Test Case 5: Invalid path ===\\n");
    listDirectory("");
    
    return 0;
}
