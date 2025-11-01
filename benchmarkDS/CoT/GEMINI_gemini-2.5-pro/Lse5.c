#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Finds and prints an element from an array based on command-line arguments.
 * It handles parsing and validation of the index.
 *
 * @param data The array of C-strings to access.
 * @param data_size The number of elements in the data array.
 * @param argc The argument count.
 * @param argv The argument vector, where argv[1] is the index.
 */
void printElementAtIndex(const char* data[], int data_size, int argc, char* argv[]) {
    // 1. Security Check: Ensure an argument is provided.
    // Note: argv[0] is the program name, so argc should be at least 2.
    if (argc < 2) {
        fprintf(stderr, "Error: No index provided.\n");
        fprintf(stderr, "Usage: please provide an integer index as an argument.\n");
        return;
    }

    char* arg = argv[1];
    char* endptr;
    long index;

    // 2. Security Check: Use strtol for safe conversion from string to long.
    index = strtol(arg, &endptr, 10);

    // Check for conversion errors:
    // - If no digits were found, endptr will not have moved.
    // - If there are non-digit characters after the number, *endptr won't be the null terminator.
    if (endptr == arg || *endptr != '\0') {
        fprintf(stderr, "Error: Argument '%s' is not a valid integer.\n", arg);
        return;
    }

    // 3. Security Check: Ensure the index is within the array bounds.
    if (index < 0 || index >= data_size) {
        fprintf(stderr, "Error: Index %ld is out of bounds. Must be between 0 and %d.\n", index, data_size - 1);
        return;
    }

    // If all checks pass, print the element.
    printf("Element at index %ld: %s\n", index, data[index]);
}


int main(int argc, char* argv[]) {
    const char* data[] = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
    int data_size = sizeof(data) / sizeof(data[0]);

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Valid index
    printf("Test Case 1 (Input: \"2\"): ");
    char* args1[] = {"./lse5", "2"};
    printElementAtIndex(data, data_size, 2, args1);

    // Test Case 2: Out-of-bounds index (too high)
    printf("Test Case 2 (Input: \"10\"): ");
    char* args2[] = {"./lse5", "10"};
    printElementAtIndex(data, data_size, 2, args2);

    // Test Case 3: Out-of-bounds index (negative)
    printf("Test Case 3 (Input: \"-1\"): ");
    char* args3[] = {"./lse5", "-1"};
    printElementAtIndex(data, data_size, 2, args3);

    // Test Case 4: Non-numeric input
    printf("Test Case 4 (Input: \"abc\"): ");
    char* args4[] = {"./lse5", "abc"};
    printElementAtIndex(data, data_size, 2, args4);
    
    // Test Case 5: No input
    printf("Test Case 5 (Input: empty): ");
    char* args5[] = {"./lse5"};
    printElementAtIndex(data, data_size, 1, args5);
    
    printf("\n--- Testing with actual command-line arguments ---\n");
    // This part will use the actual arguments passed when running the program
    if (argc > 1) {
        printf("Actual command-line input: %s\n", argv[1]);
        printElementAtIndex(data, data_size, argc, argv);
    } else {
        printf("No actual command-line arguments provided. Run like './lse5 3'\n");
    }

    return 0;
}