#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Retrieves the value from the arguments array at the index specified by argv[1].
 * 
 * @param argc The argument count.
 * @param argv The array of command-line arguments.
 * @return The string at the specified index or a static error message.
 */
const char* getValueAtIndex(int argc, char* argv[]) {
    if (argc < 2) {
        return "Error: Not enough arguments. Please provide an index.";
    }

    char* endptr;
    errno = 0; // To distinguish success/failure after call
    long index_long = strtol(argv[1], &endptr, 10);

    // Check for various conversion errors
    if ((errno == ERANGE && (index_long == LONG_MAX || index_long == LONG_MIN)) || (errno != 0 && index_long == 0)) {
        return "Error: The index provided is out of range for a long integer.";
    }
    if (endptr == argv[1] || *endptr != '\0') {
        return "Error: The index provided in the second argument must be an integer.";
    }
    // Check if the value fits in an int
    if (index_long > INT_MAX || index_long < INT_MIN) {
       return "Error: The index provided is out of range for an integer.";
    }

    int index = (int)index_long;
    
    if (index < 0 || index >= argc) {
        // Note: C doesn't have easy string concatenation like other languages.
        // A more complex implementation would use dynamic memory to build the error string.
        // For simplicity, we return a generic error message.
        return "Error: Index is out of bounds for the given arguments.";
    }

    return argv[index];
}

void run_test_case(const char* name, int argc, char* argv[]) {
    printf("\n%s: [", name);
    for (int i = 0; i < argc; ++i) {
        printf("\"%s\"%s", argv[i], (i == argc - 1 ? "" : ", "));
    }
    printf("]\n");
    printf("Output: %s\n", getValueAtIndex(argc, argv));
}

int main(int argc, char* argv[]) {
    // Example with actual command line arguments
    printf("--- Processing Actual Command Line Arguments ---\n");
    printf("Arguments received: [");
    for (int i = 0; i < argc; ++i) {
        printf("\"%s\"%s", argv[i], (i == argc - 1 ? "" : ", "));
    }
    printf("]\n");
    const char* result = getValueAtIndex(argc, argv);
    printf("Result: %s\n", result);
    printf("\n--- Running 5 Hardcoded Test Cases ---\n");
    
    // Test Case 1: Valid case
    char* test1_argv[] = {"program", "3", "apple", "banana", "cherry"};
    run_test_case("Test 1", 5, test1_argv); // Expected: cherry

    // Test Case 2: Index out of bounds
    char* test2_argv[] = {"program", "5", "one", "two"};
    run_test_case("Test 2", 4, test2_argv); // Expected: Error

    // Test Case 3: Index is not a number
    char* test3_argv[] = {"program", "foo", "bar"};
    run_test_case("Test 3", 3, test3_argv); // Expected: Error

    // Test Case 4: Not enough arguments
    char* test4_argv[] = {"program"};
    run_test_case("Test 4", 1, test4_argv); // Expected: Error

    // Test Case 5: Index points to itself
    char* test5_argv[] = {"program", "1", "arg2", "arg3"};
    run_test_case("Test 5", 4, test5_argv); // Expected: 1
    
    return 0;
}