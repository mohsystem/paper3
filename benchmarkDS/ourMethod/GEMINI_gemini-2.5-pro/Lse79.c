#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define MAX_READ_SIZE 100
#define READ_BUFFER_SIZE (MAX_READ_SIZE + 1) // 100 chars + 1 for null terminator

/**
 * @brief Allocates two buffers and reads user input into them safely.
 *
 * This function allocates two buffers of BUFFER_SIZE bytes each. It then prompts
 * the user to enter up to MAX_READ_SIZE characters from standard input for each
 * buffer, ensuring no buffer overflows occur by using fgets. It also handles
 * memory allocation errors and ensures allocated memory is freed.
 */
void allocateAndRead(void) {
    char *buf = NULL;
    char *buf1 = NULL;
    char *newline_char = NULL;

    // Allocate the first buffer
    buf = (char *)malloc(BUFFER_SIZE);
    if (buf == NULL) {
        perror("Failed to allocate memory for the first buffer");
        return;
    }
    // Initialize buffer to be safe
    memset(buf, 0, BUFFER_SIZE);

    printf("Enter up to %d characters for the first buffer:\n", MAX_READ_SIZE);
    
    // Read safely from stdin, preventing buffer overflow
    if (fgets(buf, READ_BUFFER_SIZE, stdin) == NULL) {
        if (feof(stdin)) {
            printf("End of input reached.\n");
        } else {
            perror("Failed to read from stdin for the first buffer");
        }
        free(buf);
        return;
    }

    // fgets includes the newline character, remove it if present
    newline_char = strchr(buf, '\n');
    if (newline_char != NULL) {
        *newline_char = '\0';
    } else {
        // Input was too long for fgets, so we need to clear the rest of the line
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }

    printf("First buffer content: %s\n", buf);

    // Allocate the second buffer
    buf1 = (char *)malloc(BUFFER_SIZE);
    if (buf1 == NULL) {
        perror("Failed to allocate memory for the second buffer");
        free(buf); // Clean up previously allocated memory
        return;
    }
    memset(buf1, 0, BUFFER_SIZE);

    printf("Enter up to %d characters for the second buffer:\n", MAX_READ_SIZE);

    if (fgets(buf1, READ_BUFFER_SIZE, stdin) == NULL) {
        if (feof(stdin)) {
            printf("End of input reached.\n");
        } else {
            perror("Failed to read from stdin for the second buffer");
        }
        free(buf);
        free(buf1);
        return;
    }
    
    newline_char = strchr(buf1, '\n');
    if (newline_char != NULL) {
        *newline_char = '\0';
    } else {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    
    printf("Second buffer content: %s\n", buf1);

    // Free the allocated memory
    free(buf);
    free(buf1);
}

int main(void) {
    // For an interactive program, each run with different inputs constitutes a test case.
    // The function is called once per execution.
    printf("--- Test Case 1 ---\n");
    allocateAndRead();

    // The following calls would require user interaction for each test case.
    // printf("\n--- Test Case 2 ---\n");
    // allocateAndRead();
    //
    // printf("\n--- Test Case 3 ---\n");
    // allocateAndRead();
    //
    // printf("\n--- Test Case 4 ---\n");
    // allocateAndRead();
    //
    // printf("\n--- Test Case 5 ---\n");
    // allocateAndRead();
    
    return 0;
}