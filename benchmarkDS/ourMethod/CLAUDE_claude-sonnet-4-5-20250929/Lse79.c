
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Explicit zeroing function that cannot be optimized away */
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* vptr = (volatile unsigned char*)ptr;
    while (len--) {
        *vptr++ = 0;
    }
}

void memoryAllocationMethod(char* out1, size_t out1_size, char* out2, size_t out2_size) {
    char* buf = NULL;
    char* buf1 = NULL;
    
    /* Initialize output buffers to ensure null termination */
    if (out1 != NULL && out1_size > 0) {
        memset(out1, 0, out1_size);
    }
    if (out2 != NULL && out2_size > 0) {
        memset(out2, 0, out2_size);
    }
    
    /* Allocate first buffer with size validation */
    buf = (char*)calloc(1024, 1);
    if (buf == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        goto cleanup;
    }
    
    /* Read exactly 100 characters from stdin with bounds checking */
    /* Using fgets with size limit prevents buffer overflow */
    if (fgets(buf, 101, stdin) == NULL) {
        fprintf(stderr, "Input error occurred\\n");
        goto cleanup;
    }
    
    /* Ensure null termination */
    buf[100] = '\\0';
    
    /* Safely copy to output buffer with size validation */
    if (out1 != NULL && out1_size > 0) {
        size_t copy_len = (100 < out1_size - 1) ? 100 : out1_size - 1;
        memcpy(out1, buf, copy_len);
        out1[copy_len] = '\\0';
    }
    
    /* Allocate second buffer with size validation */
    buf1 = (char*)calloc(1024, 1);
    if (buf1 == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        goto cleanup;
    }
    
    /* Read exactly 100 characters from stdin with bounds checking */
    if (fgets(buf1, 101, stdin) == NULL) {
        fprintf(stderr, "Input error occurred\\n");
        goto cleanup;
    }
    
    /* Ensure null termination */
    buf1[100] = '\\0';
    
    /* Safely copy to output buffer with size validation */
    if (out2 != NULL && out2_size > 0) {
        size_t copy_len = (100 < out2_size - 1) ? 100 : out2_size - 1;
        memcpy(out2, buf1, copy_len);
        out2[copy_len] = '\\0';
    }
    
cleanup:
    /* Clear sensitive data before freeing */
    if (buf != NULL) {
        secure_zero(buf, 1024);
        free(buf);
        buf = NULL;
    }
    if (buf1 != NULL) {
        secure_zero(buf1, 1024);
        free(buf1);
        buf1 = NULL;
    }
}

int main(void) {
    char output1[1024] = {0};
    char output2[1024] = {0};
    
    /* Test case 1: Normal input */
    printf("Test 1: Normal input\\n");
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    printf("Buffer 1 length: %zu\\n", strlen(output1));
    printf("Buffer 2 length: %zu\\n", strlen(output2));
    
    /* Test case 2: Empty input */
    printf("\\nTest 2: Empty input\\n");
    memset(output1, 0, sizeof(output1));
    memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    printf("Buffer 1 length: %zu\\n", strlen(output1));
    printf("Buffer 2 length: %zu\\n", strlen(output2));
    
    /* Test case 3: Short input */
    printf("\\nTest 3: Short input\\n");
    memset(output1, 0, sizeof(output1));
    memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    printf("Buffer 1 length: %zu\\n", strlen(output1));
    printf("Buffer 2 length: %zu\\n", strlen(output2));
    
    /* Test case 4: Exactly 100 characters */
    printf("\\nTest 4: Exactly 100 characters\\n");
    memset(output1, 0, sizeof(output1));
    memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    printf("Buffer 1 length: %zu\\n", strlen(output1));
    printf("Buffer 2 length: %zu\\n", strlen(output2));
    
    /* Test case 5: More than 100 characters */
    printf("\\nTest 5: More than 100 characters\\n");
    memset(output1, 0, sizeof(output1));
    memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    printf("Buffer 1 length: %zu\\n", strlen(output1));
    printf("Buffer 2 length: %zu\\n", strlen(output2));
    
    return 0;
}
