
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define READ_SIZE 10

char* allocateBuffer() {
    char* buffer = (char*)malloc(BUFFER_SIZE * sizeof(char));
    if (buffer != NULL) {
        memset(buffer, 0, BUFFER_SIZE);
    }
    return buffer;
}

int readCharacters(char* buffer) {
    if (buffer == NULL) {
        return -1;
    }
    
    char temp[BUFFER_SIZE];
    if (fgets(temp, BUFFER_SIZE, stdin) == NULL) {
        return -1;
    }
    
    size_t len = strlen(temp);
    if (len > 0 && temp[len - 1] == '\\n') {
        temp[len - 1] = '\\0';
        len--;
    }
    
    int charsToRead = (READ_SIZE < len) ? READ_SIZE : len;
    charsToRead = (charsToRead < BUFFER_SIZE - 1) ? charsToRead : BUFFER_SIZE - 1;
    
    strncpy(buffer, temp, charsToRead);
    buffer[charsToRead] = '\\0';
    
    return charsToRead;
}

int main() {
    // Test case 1: Normal input
    printf("Test 1 - Enter 10 characters:\\n");
    char* buf1 = allocateBuffer();
    if (buf1 != NULL) {
        int read1 = readCharacters(buf1);
        printf("Characters read: %d\\n", read1);
        free(buf1);
    }
    
    // Test case 2: Less than 10 characters
    printf("\\nTest 2 - Enter less than 10 characters:\\n");
    char* buf2 = allocateBuffer();
    if (buf2 != NULL) {
        int read2 = readCharacters(buf2);
        printf("Characters read: %d\\n", read2);
        free(buf2);
    }
    
    // Test case 3: More than 10 characters
    printf("\\nTest 3 - Enter more than 10 characters:\\n");
    char* buf3 = allocateBuffer();
    if (buf3 != NULL) {
        int read3 = readCharacters(buf3);
        printf("Characters read: %d\\n", read3);
        free(buf3);
    }
    
    // Test case 4: Empty input
    printf("\\nTest 4 - Press Enter without input:\\n");
    char* buf4 = allocateBuffer();
    if (buf4 != NULL) {
        int read4 = readCharacters(buf4);
        printf("Characters read: %d\\n", read4);
        free(buf4);
    }
    
    // Test case 5: Special characters
    printf("\\nTest 5 - Enter special characters:\\n");
    char* buf5 = allocateBuffer();
    if (buf5 != NULL) {
        int read5 = readCharacters(buf5);
        printf("Characters read: %d\\n", read5);
        free(buf5);
    }
    
    return 0;
}
