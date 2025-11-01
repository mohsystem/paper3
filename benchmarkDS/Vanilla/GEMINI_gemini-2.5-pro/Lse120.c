#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void allocateCopyAndPrint(const void* sourceData, size_t size) {
    if (sourceData == NULL && size > 0) {
        printf("Error: Source data is null but size is non-zero.\n");
        return;
    }

    // 1. Allocate the first chunk of memory on the heap.
    char* firstChunk = (char*)malloc(size);
    if (firstChunk == NULL && size > 0) {
        fprintf(stderr, "Memory allocation failed for first chunk.\n");
        return;
    }

    // Copy source data into the first chunk.
    if (size > 0) {
        memcpy(firstChunk, sourceData, size);
    }
    
    // 2. Allocate the second chunk of memory on the heap.
    char* secondChunk = (char*)malloc(size);
    if (secondChunk == NULL && size > 0) {
        fprintf(stderr, "Memory allocation failed for second chunk.\n");
        free(firstChunk); // Clean up the first chunk
        return;
    }

    // 3. Copy the contents of the first chunk into the second chunk.
    if (size > 0) {
        memcpy(secondChunk, firstChunk, size);
    }

    // 4. Print the contents of the second chunk.
    printf("Contents of the second chunk: [");
    for (size_t i = 0; i < size; ++i) {
        // In C, char is an integer type, so printing with %d shows its value.
        printf("%d", secondChunk[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");

    // 5. Free the allocated memory to prevent memory leaks.
    free(firstChunk);
    free(secondChunk);
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Regular char array
    printf("\nTest Case 1:\n");
    char data1[] = {10, 20, 30, 40, 50};
    allocateCopyAndPrint(data1, sizeof(data1));

    // Test Case 2: C-style string
    printf("\nTest Case 2:\n");
    const char* data2 = "Hello";
    allocateCopyAndPrint(data2, strlen(data2));

    // Test Case 3: Empty array
    printf("\nTest Case 3:\n");
    char data3[] = {};
    allocateCopyAndPrint(data3, sizeof(data3));

    // Test Case 4: Single element array
    printf("\nTest Case 4:\n");
    char data4[] = {127};
    allocateCopyAndPrint(data4, sizeof(data4));

    // Test Case 5: Array with negative values
    printf("\nTest Case 5:\n");
    char data5[] = {-1, -128, 0, 1, 2};
    allocateCopyAndPrint(data5, sizeof(data5));

    return 0;
}