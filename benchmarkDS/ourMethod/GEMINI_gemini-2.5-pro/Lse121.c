#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates two memory chunks, copies data, and demonstrates memory management.
 *
 * This function allocates two dynamic char arrays. It copies the content of the
 * provided string into the first, then copies the first chunk to the second.
 * It prints the second chunk's content before and after freeing the first chunk
 * to show they are independent. Finally, it frees the second chunk.
 *
 * @param initialData The C-style string to initialize the first memory chunk with.
 */
void demonstrateMemoryOperations(const char* initialData) {
    if (initialData == NULL) {
        fprintf(stderr, "Error: initialData cannot be NULL.\n");
        return;
    }
    printf("--- Demonstrating with data: \"%s\" ---\n", initialData);

    // Use the length of the string + 1 for the null terminator.
    size_t size = strlen(initialData) + 1;

    // 1. Allocate two chunks of memory.
    char* firstChunk = (char*)malloc(size);
    if (firstChunk == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for firstChunk.\n");
        return;
    }

    char* secondChunk = (char*)malloc(size);
    if (secondChunk == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for secondChunk.\n");
        free(firstChunk); // Clean up the first allocation
        return;
    }

    // Initialize the first chunk with the data.
    // memcpy is used for raw memory copy. It's safe here because we calculated the exact size needed.
    memcpy(firstChunk, initialData, size);

    // 2. Copy the contents of the first chunk into the second chunk.
    memcpy(secondChunk, firstChunk, size);

    // 3. Print the contents of the second chunk.
    printf("Content of second chunk after copy: %s\n", secondChunk);

    // 4. Free the memory allocated to the first chunk.
    free(firstChunk);
    firstChunk = NULL; // Good practice to nullify pointer after free.
    printf("First chunk freed.\n");

    // 5. Print the contents of the second chunk again.
    // The content remains because the second chunk is a separate memory area.
    printf("Content of second chunk after freeing first: %s\n", secondChunk);

    // 6. Free the memory allocated to the second chunk.
    free(secondChunk);
    secondChunk = NULL;
    printf("Second chunk freed.\n");
    printf("-------------------------------------------------\n");
}

int main() {
    // 5 Test Cases
    demonstrateMemoryOperations("Test case 1: Hello, C!");
    demonstrateMemoryOperations("Test case 2: A different string.");
    demonstrateMemoryOperations("Test case 3: Short");
    demonstrateMemoryOperations("Test case 4: A much longer string to test allocation with different sizes.");
    demonstrateMemoryOperations("Test case 5: Final test case with symbols!@#$%");

    return 0;
}