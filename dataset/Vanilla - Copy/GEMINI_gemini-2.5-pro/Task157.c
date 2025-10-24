#include <stdio.h>
#include <stdlib.h>

/**
 * Demonstrates dynamic memory allocation and deallocation in C.
 * We use 'malloc' for allocation and 'free' for deallocation.
 *
 * @param size The number of integers to allocate space for.
 */
void manageMemory(int size) {
    if (size <= 0) {
        printf("Cannot allocate memory for size %d.\n", size);
        return;
    }
    
    printf("Attempting to allocate memory for an array of %d integers.\n", size);
    
    // 1. Dynamic Allocation
    // Allocate memory for 'size' integers. sizeof(int) makes it portable.
    int* dynamicArray = (int*)malloc(size * sizeof(int));

    // Check if malloc was successful
    if (dynamicArray == NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    printf("Memory allocated successfully at address: %p\n", (void*)dynamicArray);

    // Using the allocated memory
    dynamicArray[0] = 100;
    printf("First element set to: %d\n", dynamicArray[0]);

    // 2. Dynamic Deallocation
    printf("Freeing the allocated memory.\n");
    free(dynamicArray);
    dynamicArray = NULL; // Good practice to nullify pointer after freeing

    printf("Memory for %d integers has been freed.\n\n", size);
}

int main() {
    printf("--- C Memory Management Demo ---\n");
    // 5 Test Cases
    manageMemory(10);
    manageMemory(1000);
    manageMemory(50000);
    manageMemory(0);
    manageMemory(1);
    printf("--- End of C Demo ---\n");
    return 0;
}