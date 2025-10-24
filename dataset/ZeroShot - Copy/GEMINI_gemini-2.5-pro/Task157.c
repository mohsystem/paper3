#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Demonstrates secure dynamic memory allocation and deallocation in C.
 *
 * This function manually allocates memory using malloc and deallocates it
 * using free. It includes crucial security checks:
 * 1. Verifies the return value of malloc to handle allocation failures.
 * 2. Explicitly calls free() to prevent memory leaks.
 * 3. Sets the pointer to NULL after freeing to prevent use-after-free vulnerabilities.
 *
 * @param size The number of integers to allocate.
 */
void demonstrateResourceManagement(size_t size) {
    if (size == 0) {
        printf("Size is zero. No allocation performed.\n");
        return;
    }

    // A pointer to hold the address of the allocated memory.
    int* dynamicArray = NULL;
    
    // Calculate the total bytes needed, checking for potential overflow.
    if (size > (size_t)-1 / sizeof(int)) {
        fprintf(stderr, "Error: Integer overflow detected in size calculation.\n");
        return;
    }
    size_t totalBytes = size * sizeof(int);

    // 1. Dynamic Allocation
    dynamicArray = (int*)malloc(totalBytes);

    // SECURITY: Always check if malloc succeeded.
    // If malloc fails, it returns NULL. Using a NULL pointer leads to crashes.
    if (dynamicArray == NULL) {
        fprintf(stderr, "Memory allocation failed for %zu integers.\n", size);
        return; // Exit the function safely.
    }

    printf("Successfully allocated memory for %zu integers.\n", size);

    // Use the allocated memory...
    dynamicArray[0] = 100;
    printf("First element set to: %d\n", dynamicArray[0]);

    // 2. Deallocation
    // SECURITY: Always free memory that was dynamically allocated.
    // Forgetting to free leads to memory leaks.
    free(dynamicArray);
    printf("Resource freed.\n");

    // SECURITY: Set the pointer to NULL after freeing.
    // This prevents accidental "use-after-free" or "double-free" bugs.
    dynamicArray = NULL;
}

int main() {
    printf("--- C Dynamic Memory Management ---\n");
    // 5 Test Cases
    demonstrateResourceManagement(10);
    printf("--------------------\n");
    demonstrateResourceManagement(1000);
    printf("--------------------\n");
    demonstrateResourceManagement(0);
    printf("--------------------\n");
    demonstrateResourceManagement(50000);
    printf("--------------------\n");
    // A very large allocation that will likely fail
    demonstrateResourceManagement(2000000000);
    printf("--- End of C Demo ---\n");
    return 0;
}