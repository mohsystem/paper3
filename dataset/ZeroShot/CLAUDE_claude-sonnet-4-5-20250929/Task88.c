
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void generateRandomString(char* output, int length) {
    const char* characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int charLen = strlen(characters);
    
    for (int i = 0; i < length; i++) {
        output[i] = characters[rand() % charLen];
    }
    output[length] = '\\0';
}

char* convertToUnicode(const char* input) {
    int len = strlen(input);
    char* unicode = (char*)malloc(len * 6 + 1);
    if (!unicode) return NULL;
    
    unicode[0] = '\\0';
    char temp[10];
    
    for (int i = 0; i < len; i++) {
        sprintf(temp, "\\\\u%04x", (unsigned char)input[i]);
        strcat(unicode, temp);
    }
    
    return unicode;
}

char* createTempFileWithUnicode(int stringLength) {
    srand((unsigned int)time(NULL) + rand());
    
    // Generate random string
    char* randomString = (char*)malloc(stringLength + 1);
    if (!randomString) return NULL;
    generateRandomString(randomString, stringLength);
    
    // Convert to Unicode
    char* unicodeString = convertToUnicode(randomString);
    free(randomString);
    if (!unicodeString) return NULL;
    
    // Create temporary file path
    char* tempPath = (char*)malloc(256);
    if (!tempPath) {
        free(unicodeString);
        return NULL;
    }
    
#ifdef _WIN32
    char tempDir[MAX_PATH];
    char tempFile[MAX_PATH];
    GetTempPathA(MAX_PATH, tempDir);
    GetTempFileNameA(tempDir, "tmp_", 0, tempFile);
    strcpy(tempPath, tempFile);
#else
    strcpy(tempPath, "/tmp/temp_XXXXXX.txt");
    int fd = mkstemps(tempPath, 4);
    if (fd == -1) {
        free(unicodeString);
        free(tempPath);
        return NULL;
    }
    close(fd);
#endif
    
    // Write to file
    FILE* file = fopen(tempPath, "w");
    if (!file) {
        free(unicodeString);
        free(tempPath);
        return NULL;
    }
    
    fprintf(file, "%s", unicodeString);
    fclose(file);
    free(unicodeString);
    
    return tempPath;
}

int main() {
    // Test case 1
    char* path1 = createTempFileWithUnicode(10);
    if (path1) {
        printf("Test 1 - Temp file created: %s\\n", path1);
        free(path1);
    }
    
    // Test case 2
    char* path2 = createTempFileWithUnicode(20);
    if (path2) {
        printf("Test 2 - Temp file created: %s\\n", path2);
        free(path2);
    }
    
    // Test case 3
    char* path3 = createTempFileWithUnicode(5);
    if (path3) {
        printf("Test 3 - Temp file created: %s\\n", path3);
        free(path3);
    }
    
    // Test case 4
    char* path4 = createTempFileWithUnicode(15);
    if (path4) {
        printf("Test 4 - Temp file created: %s\\n", path4);
        free(path4);
    }
    
    // Test case 5
    char* path5 = createTempFileWithUnicode(25);
    if (path5) {
        printf("Test 5 - Temp file created: %s\\n", path5);
        free(path5);
    }
    
    return 0;
}
