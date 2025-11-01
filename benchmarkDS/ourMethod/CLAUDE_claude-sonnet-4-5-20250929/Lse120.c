
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Secure zeroization function that compiler cannot optimize away */
static void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Allocate two chunks of memory, copy first to second, and return second */
char* allocateAndCopy(const char* sourceData) {
    char* firstChunk = NULL;
    char* secondChunk = NULL;
    size_t dataLen = 0;
    size_t allocSize = 0;
    
    /* Handle NULL input */
    if (sourceData == NULL) {
        char* empty = (char*)malloc(1);
        if (empty == NULL) {
            fprintf(stderr, "Memory allocation failed\\n");
            return NULL;
        }
        empty[0] = '\\0';
        return empty;
    }
    
    /* Step 1: Calculate size with bounds checking */
    dataLen = strlen(sourceData);
    
    /* Prevent integer overflow and excessive allocation (1MB limit) */
    if (dataLen > 1048576 || dataLen >= SIZE_MAX - 1) {
        fprintf(stderr, "Data size exceeds maximum allowed\\n");
        return NULL;
    }
    
    allocSize = dataLen + 1; /* +1 for null terminator */
    
    /* Step 2: Allocate first chunk */
    firstChunk = (char*)malloc(allocSize);
    if (firstChunk == NULL) {
        fprintf(stderr, "Memory allocation failed for first chunk\\n");
        return NULL;
    }
    
    /* Step 3: Copy source data to first chunk with bounds checking */
    strncpy(firstChunk, sourceData, dataLen);
    firstChunk[dataLen] = '\\0'; /* Ensure null termination */
    
    /* Step 4: Allocate second chunk */
    secondChunk = (char*)malloc(allocSize);
    if (secondChunk == NULL) {
        fprintf(stderr, "Memory allocation failed for second chunk\\n");
        secure_zero(firstChunk, allocSize);
        free(firstChunk);
        return NULL;
    }
    
    /* Step 5: Copy from first chunk to second chunk */
    memcpy(secondChunk, firstChunk, allocSize);
    
    /* Step 6: Clear and free first chunk */
    secure_zero(firstChunk, allocSize);
    free(firstChunk);
    
    return secondChunk;
}

int main(void) {
    char* result = NULL;
    
    /* Test case 1: Normal string */
    result = allocateAndCopy("Hello World");
    if (result != NULL) {
        printf("Test 1: %s\\n", result);
        secure_zero(result, strlen(result) + 1);
        free(result);
    }
    
    /* Test case 2: Empty string */
    result = allocateAndCopy("");
    if (result != NULL) {
        printf("Test 2: %s\\n", result);
        free(result);
    }
    
    /* Test case 3: String with special characters */
    result = allocateAndCopy("Test@123!#$");
    if (result != NULL) {
        printf("Test 3: %s\\n", result);
        secure_zero(result, strlen(result) + 1);
        free(result);
    }
    
    /* Test case 4: NULL input */
    result = allocateAndCopy(NULL);
    if (result != NULL) {
        printf("Test 4: %s\\n", result);
        free(result);
    }
    
    /* Test case 5: Longer string */
    result = allocateAndCopy("The quick brown fox jumps over the lazy dog");
    if (result != NULL) {
        printf("Test 5: %s\\n", result);
        secure_zero(result, strlen(result) + 1);
        free(result);
    }
    
    return 0;
}
