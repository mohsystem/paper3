
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// Secure method using POSIX directory functions
char* list_directory(const char* directory_path) {
    DIR* dir;
    struct dirent* entry;
    struct stat statbuf;
    static char result[4096];
    int offset = 0;
    
    result[0] = '\\0';
    
    // Check if directory exists
    if (stat(directory_path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        snprintf(result, sizeof(result), "Invalid directory path");
        return result;
    }
    
    dir = opendir(directory_path);
    if (dir == NULL) {
        snprintf(result, sizeof(result), "Cannot open directory");
        return result;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        int written = snprintf(result + offset, sizeof(result) - offset, 
                              "%s\\n", entry->d_name);
        if (written < 0 || offset + written >= sizeof(result)) {
            break;
        }
        offset += written;
    }
    
    closedir(dir);
    return result;
}

// Alternative method using popen with validation
char* execute_list_command(const char* directory_path) {
    struct stat statbuf;
    static char result[4096];
    char command[512];
    FILE* pipe;
    
    result[0] = '\\0';
    
    // Validate directory path
    if (stat(directory_path, &statbuf) != 0 || !S_ISDIR(statbuf.st_mode)) {
        snprintf(result, sizeof(result), "Invalid directory path");
        return result;
    }
    
    // Build command with basic validation
    snprintf(command, sizeof(command), "ls -l \\"%s\\" 2>&1", directory_path);
    
    pipe = popen(command, "r");
    if (pipe == NULL) {
        snprintf(result, sizeof(result), "Error executing command");
        return result;
    }
    
    size_t offset = 0;
    while (fgets(result + offset, sizeof(result) - offset, pipe) != NULL) {
        offset = strlen(result);
        if (offset >= sizeof(result) - 1) {
            break;
        }
    }
    
    pclose(pipe);
    return result;
}

int main() {
    printf("Test Case 1: Current directory\\n");
    printf("%s\\n", list_directory("."));
    
    printf("\\nTest Case 2: Home directory\\n");
    const char* home = getenv("HOME");
    if (home) {
        printf("%s\\n", list_directory(home));
    }
    
    printf("\\nTest Case 3: Root directory\\n");
    printf("%s\\n", list_directory("/"));
    
    printf("\\nTest Case 4: Invalid directory\\n");
    printf("%s\\n", list_directory("/nonexistent/path"));
    
    printf("\\nTest Case 5: Tmp directory\\n");
    printf("%s\\n", list_directory("/tmp"));
    
    return 0;
}
