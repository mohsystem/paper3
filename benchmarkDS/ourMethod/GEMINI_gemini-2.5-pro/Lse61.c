#include <stdio.h>
#include <stdlib.h> // For malloc and free

void demonstrateVulnerability(size_t buffer_size) {
    if (buffer_size == 0) {
        printf("Buffer size cannot be zero.\n");
        return;
    }
    printf("\n--- Demonstrating double free with buffer size: %zu ---\n", buffer_size);

    // 1. Allocate the buffer.
    char* buffer = (char*)malloc(buffer_size);
    if (buffer == NULL) {
        perror("Failed to allocate memory");
        return;
    }

    printf("Buffer allocated at: %p\n", (void*)buffer);

    // 2. DANGEROUS: Free the buffer in a loop.
    // This is a classic Double Free (CWE-415) vulnerability.
    // The first call to free() is correct. The second call is a double free,
    // which leads to undefined behavior and will likely crash the program.
    // This code is for educational purposes only.
    for (int i = 0; i < 2; ++i) {
        printf("Loop %d: Attempting to free buffer...\n", i + 1);
        free(buffer);
        // This line may not be reached on the second iteration due to a crash.
        printf("Buffer freed (iteration %d).\n", i + 1);
    }
    
    // To mitigate double-free vulnerabilities, a pointer should be set to NULL
    // immediately after being freed. `free(NULL)` is a safe no-op.
    // However, the fundamental flaw here is the program logic that attempts
    // to free the same memory region multiple times.
    
    printf("Demonstration finished. If you see this, the program did not crash as expected.\n");
}

int main() {
    printf("--- C Double Free Demonstration ---\n");
    printf("WARNING: This program demonstrates a critical memory error and is expected to crash.\n");

    size_t test_cases[] = {16, 32, 64, 128, 256};
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    for (int i = 0; i < num_test_cases; ++i) {
        printf("\n--- Test Case %d ---", i + 1);
        demonstrateVulnerability(test_cases[i]);
        if (i == 0) {
            printf("\nNOTE: The program likely crashed on the first test case.\n");
            printf("If it continues, the behavior is still undefined and highly insecure.\n");
        }
    }

    return 0;
}