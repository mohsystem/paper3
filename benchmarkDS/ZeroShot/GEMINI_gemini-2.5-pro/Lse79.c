#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Allocates two 1024-byte buffers and securely reads up to 100
 * characters from stdin into each, then frees the memory.
 */
void performAllocationAndRead() {
    // 1. Allocate first buffer.
    char *buf = (char *)malloc(1024);
    if (buf == NULL) {
        perror("Failed to allocate memory for buf");
        return;
    }
    // Defensive initialization to zero.
    memset(buf, 0, 1024);

    // 2. Read into first buffer.
    printf("Enter up to 100 characters for the first buffer: ");
    // Use fgets for safe, bounded input reading. Reads 100 chars + 1 for null.
    if (fgets(buf, 101, stdin) == NULL) {
        // Handle EOF or read error.
        perror("Error reading for first buffer");
        free(buf);
        return;
    }

    // Securely handle input that was too long. If newline is not in buffer,
    // it means the input was truncated, so we must clear the rest of the line.
    if (strchr(buf, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    } else {
        // If newline is present, remove it for cleaner string processing.
        buf[strcspn(buf, "\n")] = 0;
    }

    // 3. Allocate second buffer.
    char *buf1 = (char *)malloc(1024);
    if (buf1 == NULL) {
        perror("Failed to allocate memory for buf1");
        free(buf); // Don't leak the first buffer.
        return;
    }
    memset(buf1, 0, 1024);

    // 4. Read into second buffer.
    printf("Enter up to 100 characters for the second buffer: ");
    if (fgets(buf1, 101, stdin) == NULL) {
        perror("Error reading for second buffer");
        free(buf);
        free(buf1);
        return;
    }

    // Same robust handling for the second input.
    if (strchr(buf1, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    } else {
        buf1[strcspn(buf1, "\n")] = 0;
    }

    // Print results.
    printf("\n--- Contents ---\n");
    printf("First buffer: %s\n", buf);
    printf("Second buffer: %s\n", buf1);
    printf("----------------\n\n");

    // 5. Free all allocated memory.
    free(buf);
    free(buf1);
}

int main() {
    for (int i = 1; i <= 5; i++) {
        printf("--- Test Case %d ---\n", i);
        performAllocationAndRead();
    }
    return 0;
}