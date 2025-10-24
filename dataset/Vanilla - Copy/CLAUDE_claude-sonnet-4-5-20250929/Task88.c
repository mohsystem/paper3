
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
    const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int chars_len = strlen(chars);
    
    for (int i = 0; i < length; i++) {
        output[i] = chars[rand() % chars_len];
    }
    output[length] = '\\0';
}

char* convertToUnicode(const char* input) {
    int len = strlen(input);
    char* unicode = (char*)malloc(len * 6 + 1);
    unicode[0] = '\\0';
    
    char buffer[10];
    for (int i = 0; i < len; i++) {
        sprintf(buffer, "\\\\u%04x", (unsigned char)input[i]);
        strcat(unicode, buffer);
    }
    
    return unicode;
}

char* createTempFileWithUnicode(int stringLength) {
    // Seed random number generator
    srand(time(NULL) + rand());
    
    // Create a random string
    char* randomString = (char*)malloc(stringLength + 1);
    generateRandomString(randomString, stringLength);
    
    // Convert string to Unicode
    char* unicodeString = convertToUnicode(randomString);
    
    // Create temporary file path
    char* tempPath = (char*)malloc(256);
    
#ifdef _WIN32
    char tempPathBuf[MAX_PATH];
    char tempFileName[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPathBuf);
    GetTempFileNameA(tempPathBuf, "temp_", 0, tempFileName);
    strcpy(tempPath, tempFileName);
#else
    strcpy(tempPath, "/tmp/temp_XXXXXX.txt");
    int fd = mkstemp(tempPath);
    if (fd != -1) {
        close(fd);
    }
#endif
    
    // Write Unicode string to file
    FILE* file = fopen(tempPath, "w");
    if (file) {
        fprintf(file, "%s", unicodeString);
        fclose(file);
    }
    
    free(randomString);
    free(unicodeString);
    
    return tempPath;
}

int main() {
    // Test case 1: Create temp file with 10 character string
    printf("Test Case 1:\\n");
    char* path1 = createTempFileWithUnicode(10);
    printf("Temp file created at: %s\\n", path1);
    FILE* file1 = fopen(path1, "r");
    if (file1) {
        char buffer[1024];
        fgets(buffer, sizeof(buffer), file1);
        printf("Content: %s\\n\\n", buffer);
        fclose(file1);
    }
    
    // Test case 2: Create temp file with 20 character string
    printf("Test Case 2:\\n");
    char* path2 = createTempFileWithUnicode(20);
    printf("Temp file created at: %s\\n", path2);
    FILE* file2 = fopen(path2, "r");
    if (file2) {
        char buffer[1024];
        fgets(buffer, sizeof(buffer), file2);
        printf("Content: %s\\n\\n", buffer);
        fclose(file2);
    }
    
    // Test case 3: Create temp file with 5 character string
    printf("Test Case 3:\\n");
    char* path3 = createTempFileWithUnicode(5);
    printf("Temp file created at: %s\\n", path3);
    FILE* file3 = fopen(path3, "r");
    if (file3) {
        char buffer[1024];
        fgets(buffer, sizeof(buffer), file3);
        printf("Content: %s\\n\\n", buffer);
        fclose(file3);
    }
    
    // Test case 4: Create temp file with 15 character string
    printf("Test Case 4:\\n");
    char* path4 = createTempFileWithUnicode(15);
    printf("Temp file created at: %s\\n", path4);
    FILE* file4 = fopen(path4, "r");
    if (file4) {
        char buffer[1024];
        fgets(buffer, sizeof(buffer), file4);
        printf("Content: %s\\n\\n", buffer);
        fclose(file4);
    }
    
    // Test case 5: Create temp file with 25 character string
    printf("Test Case 5:\\n");
    char* path5 = createTempFileWithUnicode(25);
    printf("Temp file created at: %s\\n", path5);
    FILE* file5 = fopen(path5, "r");
    if (file5) {
        char buffer[2048];
        fgets(buffer, sizeof(buffer), file5);
        printf("Content: %s\\n\\n", buffer);
        fclose(file5);
    }
    
    free(path1);
    free(path2);
    free(path3);
    free(path4);
    free(path5);
    
    return 0;
}
