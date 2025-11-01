#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Retrieves the argument at the index specified by argv[1].
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of C-style strings representing the arguments.
 * @return A const pointer to the argument at the specified index, or NULL if
 *         there are not enough arguments, the index is malformed/out of range,
 *         or the index is out of bounds.
 */
const char* getArgumentByIndex(int argc, char* argv[]) {
    // Rule #4: Validate input - check if enough arguments are provided.
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments. Please provide an index.\n");
        return NULL;
    }

    char* endptr;
    errno = 0; // To distinguish success/failure after call

    // Rule #5: Use safer strtol instead of atoi to detect errors.
    long index_long = strtol(argv[1], &endptr, 10);

    // Rule #4: Validate input format.
    // Check if conversion failed (no digits found, or leftover characters).
    if (endptr == argv[1] || *endptr != '\0') {
        fprintf(stderr, "Error: The first argument must be a valid integer index.\n");
        return NULL;
    }

    // Check for other errors like overflow.
    if ((errno == ERANGE && (index_long == LONG_MAX || index_long == LONG_MIN)) || (errno != 0 && index_long == 0)) {
        perror("strtol");
        return NULL;
    }

    // Rule #1 & #4: Ensure the index is within the buffer's boundaries.
    if (index_long < 0 || index_long >= argc) {
        fprintf(stderr, "Error: Index %ld is out of bounds.\n", index_long);
        return NULL;
    }

    return argv[index_long];
}

void runTest(const char* testName, int argc, char* argv[]) {
    printf("\n%s\n", testName);
    printf("Input: ");
    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }
    printf("\n");

    const char* result = getArgumentByIndex(argc, argv);
    printf("Output: %s\n", (result != NULL) ? result : "null");
}

void runTests() {
    printf("\n--- Running 5 test cases ---\n");

    // Test Case 1: Valid case
    char* test1[] = {(char*)"program", (char*)"2", (char*)"first", (char*)"second"};
    runTest("Test Case 1: Valid index", 4, test1);

    // Test Case 2: Index out of bounds (too high)
    char* test2[] = {(char*)"program", (char*)"5", (char*)"only", (char*)"three", (char*)"args"};
    runTest("Test Case 2: Index out of bounds (high)", 5, test2);

    // Test Case 3: Negative index
    char* test3[] = {(char*)"program", (char*)"-1", (char*)"a", (char*)"b"};
    runTest("Test Case 3: Negative index", 4, test3);
    
    // Test Case 4: Invalid index format (not a number)
    char* test4[] = {(char*)"program", (char*)"foo", (char*)"bar"};
    runTest("Test Case 4: Invalid index format", 3, test4);

    // Test Case 5: Not enough arguments
    char* test5[] = {(char*)"program"};
    runTest("Test Case 5: Not enough arguments", 1, test5);

    printf("\n--- Tests finished ---\n");
}

int main(int argc, char* argv[]) {
    printf("--- Running with actual command line arguments ---\n");
    const char* result = getArgumentByIndex(argc, argv);
    if (result != NULL) {
        printf("Value at specified index: %s\n", result);
    } else {
        printf("Failed to retrieve argument.\n");
        // Rule #8: Fail closed
        // return 1; // Commented out to allow tests to run
    }
    printf("------------------------------------------------\n");
    
    runTests();

    return 0;
}