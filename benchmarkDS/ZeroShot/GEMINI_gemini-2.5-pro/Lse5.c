#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// Predefined data array
static const int data[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
static const size_t data_size = sizeof(data) / sizeof(data[0]);

/**
 * @brief Securely accesses an element in a predefined array based on a string index.
 * Handles parsing errors and bounds checking.
 * @param indexStr The string representation of the index to access.
 */
void processIndex(const char* indexStr) {
    char* endptr;
    errno = 0; // Reset errno before the call

    long index = strtol(indexStr, &endptr, 10);

    // Securely check for conversion errors
    if (endptr == indexStr) {
        fprintf(stderr, "Error: Invalid input. No digits were found.\n");
        return;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid input. Trailing characters found: %s\n", endptr);
        return;
    }
    if ((errno == ERANGE && (index == LONG_MAX || index == LONG_MIN)) || (errno != 0 && index == 0)) {
        fprintf(stderr, "Error: Number is out of range for a long integer.\n");
        return;
    }

    // Secure bounds checking
    if (index >= 0 && index < (long)data_size) {
        printf("Element at index %ld: %d\n", index, data[index]);
    } else {
        fprintf(stderr, "Error: Index %ld is out of bounds.\n", index);
    }
}

/**
 * @brief Runs a series of predefined test cases.
 */
void runTests() {
    printf("--- Running Test Cases ---\n");
    
    // Test Case 1: Valid index
    printf("Test 1 (Input: \"3\"): ");
    processIndex("3");

    // Test Case 2: Negative index (out of bounds)
    printf("Test 2 (Input: \"-1\"): ");
    processIndex("-1");
    
    // Test Case 3: Index equal to array size (out of bounds)
    printf("Test 3 (Input: \"10\"): ");
    processIndex("10");

    // Test Case 4: Last valid index
    printf("Test 4 (Input: \"9\"): ");
    processIndex("9");
    
    // Test Case 5: Non-numeric input
    printf("Test 5 (Input: \"abc\"): ");
    processIndex("abc");

    printf("--- Test Cases Finished ---\n");
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        // Process the command line argument
        processIndex(argv[1]);
    } else {
        // Run built-in test cases if no/wrong number of arguments is provided
        printf("Usage: %s <index>\n", argv[0]);
        printf("No command line argument provided. Running default test cases.\n");
        runTests();
    }
    return 0;
}