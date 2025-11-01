
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

void readInput(char* buf) {
    if (buf == NULL) return;
    
    for (int i = 0; i < READ_SIZE; i++) {
        int ch = getchar();
        if (ch == EOF) break;
        buf[i] = (char)ch;
    }
}

int main() {
    // Test case 1: Basic allocation and read
    printf("Test 1: Allocate buffer and read input\\n");
    char* buffer1 = allocateBuffer();
    printf("Buffer allocated: %s\\n", buffer1 != NULL ? "PASS" : "FAIL");
    free(buffer1);
    
    // Test case 2: Verify buffer allocation
    printf("\\nTest 2: Verify buffer allocation\\n");
    char* buffer2 = allocateBuffer();
    printf("Buffer allocated: %s\\n", buffer2 != NULL ? "PASS" : "FAIL");
    free(buffer2);
    
    // Test case 3: Multiple allocations
    printf("\\nTest 3: Multiple buffer allocations\\n");
    char* buffer3 = allocateBuffer();
    char* buffer4 = allocateBuffer();
    printf("Buffer 3: %s\\n", buffer3 != NULL ? "PASS" : "FAIL");
    printf("Buffer 4: %s\\n", buffer4 != NULL ? "PASS" : "FAIL");
    free(buffer3);
    free(buffer4);
    
    // Test case 4: Buffer initialization
    printf("\\nTest 4: Buffer initialization\\n");
    char* buffer5 = allocateBuffer();
    int initialized = 1;
    for (int i = 0; i < 10; i++) {
        if (buffer5[i] != '\\0') {
            initialized = 0;
            break;
        }
    }
    printf("Buffer initialized to zero: %s\\n", initialized ? "PASS" : "FAIL");
    free(buffer5);
    
    // Test case 5: Buffer independence
    printf("\\nTest 5: Buffer independence\\n");
    char* buffer6 = allocateBuffer();
    char* buffer7 = allocateBuffer();
    buffer6[0] = 'A';
    buffer7[0] = 'B';
    printf("Buffers are independent: %s\\n", buffer6[0] != buffer7[0] ? "PASS" : "FAIL");
    free(buffer6);
    free(buffer7);
    
    return 0;
}
