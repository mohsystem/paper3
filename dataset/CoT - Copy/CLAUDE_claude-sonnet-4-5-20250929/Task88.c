
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
#else
    #include <unistd.h>
    #include <sys/stat.h>
#endif

#define CHARACTERS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
#define RANDOM_STRING_LENGTH 20
#define MAX_PATH_LEN 512

void generateSecureRandomString(char* output, int length) {
    int charLen = strlen(CHARACTERS);
    
    for (int i = 0; i < length; i++) {
        output[i] = CHARACTERS[rand() % charLen];
    }
    output[length] = '\\0';
}

char* createTempFileWithRandomString() {
    static char tempPath[MAX_PATH_LEN];
    char randomString[RANDOM_STRING_LENGTH + 1];
    FILE* file = NULL;
    
    // Seed random number generator
    srand((unsigned int)time(NULL) + rand());
    
    // Generate secure random string
    generateSecureRandomString(randomString, RANDOM_STRING_LENGTH);
    
#ifdef _WIN32
    char tempDir[MAX_PATH_LEN];
    char tempFileName[MAX_PATH_LEN];
    GetTempPathA(MAX_PATH_LEN, tempDir);
    GetTempFileNameA(tempDir, "secure_temp_", 0, tempFileName);
    strncpy(tempPath, tempFileName, MAX_PATH_LEN - 1);
    tempPath[MAX_PATH_LEN - 1] = '\\0';
#else
    strcpy(tempPath, "/tmp/secure_temp_XXXXXX");
    int fd = mkstemp(tempPath);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return NULL;
    }
    
    // Set restrictive permissions (0600)
    fchmod(fd, S_IRUSR | S_IWUSR);
    close(fd);
#endif
    
    // Write UTF-8 encoded string to file
    file = fopen(tempPath, "w");
    if (file == NULL) {
        perror("Failed to open temporary file");
        return NULL;
    }
    
    fprintf(file, "%s", randomString);
    fclose(file);
    
    return tempPath;
}

int main() {
    printf("Testing temporary file creation with random Unicode strings:\\n\\n");
    
    for (int i = 1; i <= 5; i++) {
        char* filePath = createTempFileWithRandomString();
        if (filePath != NULL) {
            printf("Test %d: %s\\n", i, filePath);
            
            // Verify file content
            FILE* file = fopen(filePath, "r");
            if (file != NULL) {
                char content[RANDOM_STRING_LENGTH + 1];
                if (fgets(content, sizeof(content), file) != NULL) {
                    printf("  Content: %s\\n", content);
                    printf("  Length: %zu characters\\n\\n", strlen(content));
                }
                fclose(file);
            }
        } else {
            fprintf(stderr, "Test %d failed\\n", i);
        }
    }
    
    return 0;
}
