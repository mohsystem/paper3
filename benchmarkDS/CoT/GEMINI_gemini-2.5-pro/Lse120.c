#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Allocates two memory chunks, copies the content of the first
 * to the second, and then prints the second chunk.
 *
 * @param data A pointer to a null-terminated C-style string.
 */
void memoryCopyAndPrint(const char* data) {
    if (data == NULL) {
        fprintf(stderr, "Error: Input data cannot be null.\n");
        return;
    }
    
    // Include space for the null terminator
    size_t size = strlen(data) + 1;

    // 1. Allocate two chunks of memory using malloc.
    char* sourceChunk = (char*)malloc(size);
    char* destinationChunk = (char*)malloc(size);

    // Security check: Ensure memory allocation was successful.
    if (sourceChunk == NULL || destinationChunk == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        free(sourceChunk);      // free() is safe to call on NULL
        free(destinationChunk);
        return;
    }

    // Populate the first chunk with the input data.
    // memcpy is used for raw memory copy. It's safe here because we calculated
    // the exact size required.
    memcpy(sourceChunk, data, size);

    // 2. Copy the contents of the first chunk into the second chunk.
    memcpy(destinationChunk, sourceChunk, size);

    // 3. Print the contents of the second chunk.
    printf("Contents of the second chunk: %s\n", destinationChunk);

    // Security best practice: Deallocate the memory to prevent memory leaks.
    free(sourceChunk);
    free(destinationChunk);
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Regular string
    memoryCopyAndPrint("Hello, C!");

    // Test Case 2: String with numbers and symbols
    memoryCopyAndPrint("123 Test Cases!@#");

    // Test Case 3: Longer string
    memoryCopyAndPrint("This is a longer test string to check allocation.");

    // Test Case 4: Single character
    memoryCopyAndPrint("A");

    // Test Case 5: Empty string
    memoryCopyAndPrint("");

    return 0;
}