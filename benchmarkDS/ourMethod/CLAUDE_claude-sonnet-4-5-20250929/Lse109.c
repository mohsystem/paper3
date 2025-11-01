
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

// Security: Validate username contains only safe characters
// Returns 1 if valid, 0 if invalid
int isValidUsername(const char* username) {
    if (!username || !*username) {
        return 0;
    }
    
    size_t len = strlen(username);
    
    // Security: Limit username length to prevent resource exhaustion
    if (len > 32) {
        return 0;
    }
    
    // Security: Check each character is alphanumeric, underscore, hyphen, or period
    for (size_t i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

// Security: Read /etc/passwd directly instead of executing shell commands
// This prevents command injection (CWE-78) by avoiding system() calls
// Returns allocated string (caller must free) or NULL on error
char* getUserInfo(const char* username) {
    if (!username || !*username) {
        char* err = malloc(32);
        if (err) {
            snprintf(err, 32, "Error: Username cannot be empty");
        }
        return err;
    }
    
    // Security: Validate username format
    if (!isValidUsername(username)) {
        char* err = malloc(64);
        if (err) {
            snprintf(err, 64, "Error: Invalid username format");
        }
        return err;
    }
    
    const char* passwdPath = "/etc/passwd";
    
    // Security: Open file first with O_NOFOLLOW to prevent symlink attacks
    int fd = open(passwdPath, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        char* err = malloc(64);
        if (err) {
            snprintf(err, 64, "Error: Cannot open /etc/passwd");
        }
        return err;
    }
    
    // Security: Validate that the opened file is a regular file (prevents TOCTOU)
    struct stat fileStat;
    if (fstat(fd, &fileStat) != 0 || !S_ISREG(fileStat.st_mode)) {
        close(fd);
        char* err = malloc(64);
        if (err) {
            snprintf(err, 64, "Error: /etc/passwd is not a regular file");
        }
        return err;
    }
    
    // Security: Check file size to prevent excessive memory allocation
    if (fileStat.st_size > 10 * 1024 * 1024) {
        close(fd);
        char* err = malloc(64);
        if (err) {
            snprintf(err, 64, "Error: File too large");
        }
        return err;
    }
    
    // Security: Use fdopen to work with the already-opened file descriptor
    FILE* file = fdopen(fd, "r");
    if (!file) {
        close(fd);
        char* err = malloc(64);
        if (err) {
            snprintf(err, 64, "Error: Cannot read /etc/passwd");
        }
        return err;
    }
    
    // Security: Allocate result buffer with size limit
    size_t resultSize = 4096;
    char* result = malloc(resultSize);
    if (!result) {
        fclose(file);
        return NULL;
    }
    result[0] = '\\0';
    
    char line[1024]; // Security: Fixed size buffer
    int found = 0;
    int lineCount = 0;
    const int maxLines = 10000; // Security: Limit lines read
    size_t usernameLen = strlen(username);
    
    // Security: Use fgets for bounds-checked reading
    while (fgets(line, sizeof(line), file) && lineCount < maxLines) {
        lineCount++;
        
        // Security: Ensure null termination
        line[sizeof(line) - 1] = '\\0';
        
        // Remove newline
        size_t lineLen = strlen(line);
        if (lineLen > 0 && line[lineLen - 1] == '\\n') {
            line[lineLen - 1] = '\\0';
            lineLen--;
        }
        
        // Parse line format: username:password:uid:gid:gecos:home:shell
        // Security: Check if line starts with username followed by colon
        if (lineLen > usernameLen && 
            strncmp(line, username, usernameLen) == 0 && 
            line[usernameLen] == ':') {
            
            // Security: Check if result buffer has space
            size_t currentLen = strlen(result);
            size_t needed = currentLen + lineLen + 2; // +2 for \
 and \\0
            
            if (needed > resultSize) {
                // Security: Grow buffer but limit maximum size
                if (resultSize >= 64 * 1024) { // 64KB limit
                    break;
                }
                resultSize *= 2;
                char* newResult = realloc(result, resultSize);
                if (!newResult) {
                    break;
                }
                result = newResult;
            }
            
            strcat(result, line);
            strcat(result, "\\n");
            found = 1;
        }
    }
    
    fclose(file); // Also closes fd
    
    if (!found) {
        snprintf(result, resultSize, "User not found: %s", username);
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    const char* testUsers[] = {"root", "daemon", "bin", "nonexistent", "test.user"};
    
    printf("=== User Information Lookup ===\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        printf("Looking up user: %s\\n", testUsers[i]);
        char* info = getUserInfo(testUsers[i]);
        if (info) {
            printf("%s\\n", info);
            free(info); // Security: Free allocated memory
        }
        printf("---\\n");
    }
    
    // If command line argument provided, use it
    if (argc > 1) {
        printf("\\nCommand line lookup: %s\\n", argv[1]);
        char* info = getUserInfo(argv[1]);
        if (info) {
            printf("%s\\n", info);
            free(info); // Security: Free allocated memory
        }
    }
    
    return 0;
}
