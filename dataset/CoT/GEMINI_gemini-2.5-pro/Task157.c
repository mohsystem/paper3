#include <stdio.h>
#include <stdlib.h>

/**
 * Demonstrates dynamic memory allocation and deallocation in C.
 * We use 'malloc' for allocation and 'free' for deallocation.
 * It's crucial to deallocate memory to prevent memory leaks.
 *
 * @param size The number of integers to allocate space for.
 */
void manageMemory(int size) {
    if (size <= 0) {
        printf("Size must be positive. Cannot allocate memory for size: %d\n", size);
        return;
    }
    printf("--- C: Managing memory for size %d ---\n", size);

    // 1. Dynamic Allocation: Using 'malloc' to allocate a block of memory on the heap.
    // sizeof(int) ensures portability across different systems.
    int* dynamicArray = (int*)malloc(size * sizeof(int));

    // Security Check: Always check if malloc returned NULL (indicating failure).
    if (dynamicArray == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        printf("------------------------------------------\n\n");
        return;
    }
    printf("Allocated an array of %d integers at address: %p\n", size, (void*)dynamicArray);

    // 2. Using the allocated memory
    for (int i = 0; i < size; ++i) {
        dynamicArray[i] = i * 10;
    }
    printf("Successfully used the allocated memory.\n");

    // 3. Deallocation: Freeing the memory using 'free'.
    free(dynamicArray);
    printf("Memory deallocated successfully.\n");

    // Security Best Practice: Set pointer to NULL to prevent use-after-free vulnerabilities.
    dynamicArray = NULL;
    printf("Pointer set to NULL.\n");
    printf("------------------------------------------\n\n");
}

int main() {
    // 5 test cases
    manageMemory(10);
    manageMemory(100);
    manageMemory(5);
    manageMemory(0); // Test case for invalid size
    manageMemory(1000);
    return 0;
}