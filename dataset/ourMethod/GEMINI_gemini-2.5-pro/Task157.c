#include <stdio.h>
#include <stdlib.h> // For malloc, free
#include <stdint.h> // For SIZE_MAX

/**
 * @brief Demonstrates C-style dynamic allocation and deallocation.
 *
 * This function allocates a block of memory using malloc, checks for
 * allocation failure, uses the memory, and then explicitly frees it.
 *
 * @param size The number of integers to allocate.
 */
void demonstrateAllocation(size_t size) {
    if (size == 0) {
        printf("Allocating a zero-sized array.\n");
    }

    // Check for potential integer overflow before multiplication
    if (size > 0 && size > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Error: Requested size is too large and would cause an integer overflow.\n");
        return;
    }

    printf("Attempting to allocate memory for %zu integers...\n", size);

    int *data = NULL;

    // 1. Dynamic Allocation
    data = (int*)malloc(size * sizeof(int));

    // ALWAYS check the return value of malloc
    if (data == NULL) {
        // Handle allocation failure
        if (size > 0) {
            fprintf(stderr, "Error: Memory allocation failed for size %zu.\n", size);
        } else {
             // malloc(0) behavior is implementation-defined; it might return NULL
             printf("malloc(0) returned NULL, which is a valid behavior.\n");
        }
        printf("----------------------------------------\n");
        return;
    }

    printf("Successfully allocated %zu bytes.\n", size * sizeof(int));

    // 2. Use the allocated memory
    for (size_t i = 0; i < size; ++i) {
        data[i] = i * 2;
    }
    
    if (size > 0) {
        printf("Used memory. First element: %d, Last element: %d\n", data[0], data[size - 1]);
    } else {
        printf("Used memory. Array is empty.\n");
    }


    // 3. Deallocation
    free(data);
    printf("Memory has been deallocated.\n");
    
    // Good practice: set pointer to NULL to prevent use-after-free
    data = NULL;
    
    printf("----------------------------------------\n");
}

int main() {
    printf("--- C Dynamic Memory Demonstration ---\n");
    // Test case 1: A small, valid size
    demonstrateAllocation(10);
    // Test case 2: A larger size
    demonstrateAllocation(100000);
    // Test case 3: A zero size
    demonstrateAllocation(0);
    // Test case 4: A large size
    demonstrateAllocation(10000000);
    // Test case 5: A very large size that will likely fail
    demonstrateAllocation(SIZE_MAX / sizeof(int));

    return 0;
}