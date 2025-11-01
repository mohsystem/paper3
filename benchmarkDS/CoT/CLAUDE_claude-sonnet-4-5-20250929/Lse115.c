
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

// SECURE: Uses POSIX dirent API instead of executing shell commands
void listDirectory(const char* directoryPath) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    
    // Validate input
    if (directoryPath == NULL || strlen(directoryPath) == 0) {
        printf("Invalid directory path\\n");
        return;
    }
    
    // Check if path is a directory
    if (stat(directoryPath, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        printf("Invalid directory: %s\\n", directoryPath);
        return;
    }
    
    dir = opendir(directoryPath);
    if (dir == NULL) {
        perror("Error opening directory");
        return;
    }
    
    printf("Contents of %s:\\n", directoryPath);
    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf("%s\\n", entry->d_name);
        }
    }
    
    closedir(dir);
}

int main() {
    // Test cases using safe directory listing
    printf("Test 1: Current directory\\n");
    listDirectory(".");
    
    printf("\\nTest 2: Parent directory\\n");
    listDirectory("..");
    
    printf("\\nTest 3: Temp directory\\n");
    listDirectory("/tmp");
    
    printf("\\nTest 4: Home directory\\n");
    listDirectory(getenv("HOME"));
    
    printf("\\nTest 5: Invalid directory\\n");
    listDirectory("/nonexistent/path");
    
    return 0;
}
