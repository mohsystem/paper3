#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // For errno
#include <limits.h> // For LONG_MIN, LONG_MAX

/**
 * @brief Allocates a buffer of the specified size and prints its contents.
 *
 * This function securely parses a string to a number, validates the size,
 * allocates a zero-initialized buffer, prints its contents, and frees it.
 *
 * @param size_str The desired buffer size as a string.
 */
void allocateAndPrint(const char* size_str) {
    printf("--- Processing input: \"%s\" ---\n", size_str);
    char* endptr;
    long parsed_size;

    // 1. Securely parse the string to a long
    errno = 0; // Reset errno before the call
    parsed_size = strtol(size_str, &endptr, 10);

    // 2. Validate the input
    // Check for conversion errors (e.g., "hello")
    if (endptr == size_str || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid number format. Please provide an integer.\n");
    }
    // Check for out-of-range errors (e.g., a number too large for long)
    else if (errno == ERANGE) {
        fprintf(stderr, "Error: Number is out of range.\n");
    }
    // Check for negative size
    else if (parsed_size < 0) {
        fprintf(stderr, "Error: Buffer size cannot be negative.\n");
    } else {
        size_t size = (size_t)parsed_size;
        printf("Allocating buffer of size: %zu\n", size);

        // 3. Allocate the buffer using calloc for zero-initialization and safety
        // calloc(count, element_size) helps prevent integer overflow on multiplication
        char *buffer = (char*)calloc(size, sizeof(char));

        // CRITICAL: Check if allocation failed
        if (buffer == NULL && size > 0) {
            fprintf(stderr, "Error: Failed to allocate buffer. Memory limit exceeded.\n");
        } else {
            // 4. Print the contents of the buffer
            printf("Buffer contents: [");
            for (size_t i = 0; i < size; ++i) {
                printf("%d%s", buffer[i], (i == size - 1) ? "" : ", ");
            }
            printf("]\n");

            // 5. Free the allocated memory
            free(buffer);
        }
    }
    printf("--- Finished processing ---\n\n");
}

/**
 * @brief Main entry point. Processes command-line argument or runs test cases.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit code.
 */
int main(int argc, char* argv[]) {
    // Lse60 is the Java class name. C doesn't have classes.

    if (argc == 2) {
        // Process the command-line argument if provided
        printf("Processing command-line argument...\n");
        allocateAndPrint(argv[1]);
    } else {
        // Run built-in test cases if no command-line argument is given
        printf("No command-line argument found. Running test cases...\n");

        // Test Case 1: Valid positive size
        allocateAndPrint("10");

        // Test Case 2: Zero size
        allocateAndPrint("0");

        // Test Case 3: Negative size (invalid)
        allocateAndPrint("-5");

        // Test Case 4: Non-numeric input (invalid)
        allocateAndPrint("hello");

        // Test Case 5: Large size (may fail allocation)
        allocateAndPrint("2000000000"); // 2 GB
    }

    return 0;
}