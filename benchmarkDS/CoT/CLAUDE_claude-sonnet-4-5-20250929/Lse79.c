
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* buf1;
    char* buf2;
} BufferResult;

BufferResult allocateAndReadBuffers(const char* input1, const char* input2) {
    // Security: Define buffer size constants
    const int BUFFER_SIZE = 1024;
    const int READ_SIZE = 100;
    
    BufferResult result;
    result.buf1 = NULL;
    result.buf2 = NULL;
    
    // Allocate first buffer
    char* buf = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed for buf\\n");
        return result;
    }
    
    // Allocate second buffer
    char* buf1 = (char*)calloc(BUFFER_SIZE, sizeof(char));
    if (buf1 == NULL) {
        fprintf(stderr, "Memory allocation failed for buf1\\n");
        free(buf);
        return result;
    }
    
    // Security: Validate and limit input size to prevent overflow
    if (input1 != NULL) {
        size_t len = strlen(input1);
        size_t copy_len = (len > READ_SIZE) ? READ_SIZE : len;
        strncpy(buf, input1, copy_len);
        buf[copy_len] = '\\0'; // Ensure null termination
        
        result.buf1 = (char*)malloc((copy_len + 1) * sizeof(char));
        if (result.buf1 != NULL) {
            strcpy(result.buf1, buf);
        }
    }
    
    if (input2 != NULL) {
        size_t len = strlen(input2);
        size_t copy_len = (len > READ_SIZE) ? READ_SIZE : len;
        strncpy(buf1, input2, copy_len);
        buf1[copy_len] = '\\0'; // Ensure null termination
        
        result.buf2 = (char*)malloc((copy_len + 1) * sizeof(char));
        if (result.buf2 != NULL) {
            strcpy(result.buf2, buf1);
        }
    }
    
    // Free temporary buffers
    free(buf);
    free(buf1);
    
    return result;
}

void freeBufferResult(BufferResult* result) {
    if (result->buf1 != NULL) {
        free(result->buf1);
        result->buf1 = NULL;
    }
    if (result->buf2 != NULL) {
        free(result->buf2);
        result->buf2 = NULL;
    }
}

int main() {
    // Test case 1: Normal input
    BufferResult result1 = allocateAndReadBuffers("Hello World", "Test Input");
    printf("Test 1 - Buf1: %s, Buf2: %s\\n", 
           result1.buf1 ? result1.buf1 : "", result1.buf2 ? result1.buf2 : "");
    freeBufferResult(&result1);
    
    // Test case 2: Empty strings
    BufferResult result2 = allocateAndReadBuffers("", "");
    printf("Test 2 - Buf1: '%s', Buf2: '%s'\\n", 
           result2.buf1 ? result2.buf1 : "", result2.buf2 ? result2.buf2 : "");
    freeBufferResult(&result2);
    
    // Test case 3: NULL inputs
    BufferResult result3 = allocateAndReadBuffers(NULL, NULL);
    printf("Test 3 - Buf1: '%s', Buf2: '%s'\\n", 
           result3.buf1 ? result3.buf1 : "", result3.buf2 ? result3.buf2 : "");
    freeBufferResult(&result3);
    
    // Test case 4: Input exceeding 100 characters (should be truncated)
    char longString[200];
    memset(longString, 'A', 150);
    longString[150] = '\\0';
    BufferResult result4 = allocateAndReadBuffers(longString, longString);
    printf("Test 4 - Buf1 length: %lu, Buf2 length: %lu\\n", 
           result4.buf1 ? strlen(result4.buf1) : 0, 
           result4.buf2 ? strlen(result4.buf2) : 0);
    freeBufferResult(&result4);
    
    // Test case 5: Exactly 100 characters
    char exactString[110];
    memset(exactString, 'B', 100);
    exactString[100] = '\\0';
    BufferResult result5 = allocateAndReadBuffers(exactString, exactString);
    printf("Test 5 - Buf1 length: %lu, Buf2 length: %lu\\n", 
           result5.buf1 ? strlen(result5.buf1) : 0, 
           result5.buf2 ? strlen(result5.buf2) : 0);
    freeBufferResult(&result5);
    
    return 0;
}
