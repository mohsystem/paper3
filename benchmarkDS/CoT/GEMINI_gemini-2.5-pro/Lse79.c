#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This function clears the input buffer to prevent leftover characters
// from being read in subsequent prompts.
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/**
 * Allocates two 1024-byte buffers, and for each, reads up to 100 characters
 * from standard input.
 */
void performAllocationAndRead() {
    // The first method allocates 1024 bytes of memory.
    char *buf = (char *)malloc(1024);
    if (buf == NULL) {
        perror("Failed to allocate memory for buf");
        return;
    }

    // The second method allocates another 1024 bytes of memory.
    char *buf1 = (char *)malloc(1024);
    if (buf1 == NULL) {
        perror("Failed to allocate memory for buf1");
        free(buf); // Clean up the first allocation
        return;
    }

    printf("Enter up to 100 characters for the first buffer:\n");
    // Use fgets to safely read input. It reads at most n-1 characters,
    // so we provide a size of 101 to read up to 100 characters.
    // It also adds a null terminator, preventing buffer overflows.
    if (fgets(buf, 101, stdin) != NULL) {
        // Remove the newline character if it was read by fgets
        buf[strcspn(buf, "\n")] = '\0';
    } else {
        printf("End of input or error occurred.\n");
    }
    // If the input was longer than 100 chars, the rest remains in the buffer.
    // We clear it to not affect the next read.
    clear_input_buffer();


    printf("Enter up to 100 characters for the second buffer:\n");
    if (fgets(buf1, 101, stdin) != NULL) {
        buf1[strcspn(buf1, "\n")] = '\0';
    } else {
        printf("End of input or error occurred.\n");
    }
    clear_input_buffer();

    printf("Content of first buffer: %s\n", buf);
    printf("Content of second buffer: %s\n", buf1);

    // CRITICAL: Free the allocated memory to prevent memory leaks.
    free(buf);
    free(buf1);
}

int main() {
    // Main function with 5 test cases.
    for (int i = 1; i <= 5; ++i) {
        printf("\n--- Test Case %d ---\n", i);
        performAllocationAndRead();
    }
    printf("\nAll test cases finished.\n");
    return 0;
}