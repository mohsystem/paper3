
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

int readCharacters(char* buf) {
    if (buf == NULL) {
        return -1;
    }
    
    int charsRead = 0;
    for (int i = 0; i < READ_SIZE && i < BUFFER_SIZE; i++) {
        int c = getchar();
        if (c == EOF) {
            break;
        }
        buf[i] = (char)c;
        charsRead++;
    }
    
    return charsRead;
}

int main() {
    printf("Test Case 1:\\n");
    char* buffer1 = allocateBuffer();
    printf("Buffer allocated with size: %d\\n", BUFFER_SIZE);
    free(buffer1);
    
    printf("\\nTest Case 2:\\n");
    char* buffer2 = allocateBuffer();
    printf("Enter 10 characters: \\n");
    int read = readCharacters(buffer2);
    printf("Characters read: %d\\n", read);
    free(buffer2);
    
    printf("\\nTest Case 3: Reading into buffer\\n");
    char* buffer3 = allocateBuffer();
    for (int i = 0; i < READ_SIZE; i++) {
        buffer3[i] = 'A' + i;
    }
    printf("Pre-filled buffer with test data\\n");
    free(buffer3);
    
    printf("\\nTest Case 4: Check buffer bounds\\n");
    char* buffer4 = allocateBuffer();
    printf("Buffer can safely hold %d characters: %d\\n", READ_SIZE, (BUFFER_SIZE >= READ_SIZE));
    free(buffer4);
    
    printf("\\nTest Case 5: Null buffer check\\n");
    int result = readCharacters(NULL);
    printf("Reading into null buffer returns: %d\\n", result);
    
    return 0;
}
