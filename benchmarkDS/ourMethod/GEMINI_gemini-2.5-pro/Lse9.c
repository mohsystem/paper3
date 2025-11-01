#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Retrieves a command-line argument by an index specified in the first argument.
 * 
 * @param argc The argument count.
 * @param argv The argument vector. argv[1] is expected to be the index.
 * @return A pointer to the argument string on success, or NULL on failure.
 *         Error messages are printed to stderr on failure.
 */
const char* getArgumentAtIndex(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Error: No index provided.\n");
        return NULL;
    }

    char* index_str = argv[1];
    char* endptr;
    errno = 0; // Reset errno before the call

    long target_index_long = strtol(index_str, &endptr, 10);

    // Check for conversion errors reported by strtol
    if ((errno == ERANGE && (target_index_long == LONG_MAX || target_index_long == LONG_MIN))
        || (errno != 0 && target_index_long == 0)) {
        fprintf(stderr, "Error: Index '%s' is out of range for a long integer.\n", index_str);
        return NULL;
    }

    // Check if the entire string was consumed, indicating a valid integer
    if (endptr == index_str || *endptr != '\0') {
        fprintf(stderr, "Error: Index '%s' is not a valid integer.\n", index_str);
        return NULL;
    }
    
    // We enforce positive indices, as argv[0] is the program name.
    if (target_index_long <= 0 || target_index_long >= INT_MAX) {
       fprintf(stderr, "Error: Index must be a positive integer that fits within standard argument count limits.\n");
       return NULL;
    }
    
    int target_index = (int)target_index_long;

    if (target_index >= argc) {
        fprintf(stderr, "Error: Index %d is out of bounds. There are only %d arguments.\n", target_index, argc);
        return NULL;
    }

    return argv[target_index];
}

// Helper function to run a single test case and print its output.
void run_test_case(const char* name, int argc, char* argv[]) {
    printf("%s ", name);
    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    const char* result = getArgumentAtIndex(argc, argv);
    if (result) {
        printf("Output: %s\n", result);
    } else {
        printf("Output: <error message printed to stderr>\n");
    }
    printf("\n");
}

int main(int argc, char* argv[]) {
    // To run with actual command-line arguments, compile and execute:
    // gcc your_program.c -o program -Wall -Wextra -pedantic
    // ./program 2 zero one two three  (should print "one")
    // ./program 1 one two             (should print "1")
    // ./program 5 1 2 3             (should print error)
    // ./program -1 1 2 3            (should print error)
    // ./program 0 1 2 3             (should print error)
    // ./program abc 1 2 3           (should print error)

    printf("--- Running with actual command line arguments ---\n");
    const char* result = getArgumentAtIndex(argc, argv);
    if (result) {
        printf("Output: %s\n", result);
    }
    printf("------------------------------------------------\n\n");
    
    printf("--- Running with simulated test cases ---\n");
    
    // Test Case 1: Valid index
    char* test1_argv[] = {(char*)"./program", (char*)"3", (char*)"apple", (char*)"banana", (char*)"cherry"};
    run_test_case("Test Case 1:", sizeof(test1_argv)/sizeof(char*), test1_argv); // Expected: banana

    // Test Case 2: Index out of bounds (too high)
    char* test2_argv[] = {(char*)"./program", (char*)"3", (char*)"dog", (char*)"cat"};
    run_test_case("Test Case 2:", sizeof(test2_argv)/sizeof(char*), test2_argv); // Expected: Error

    // Test Case 3: Invalid index (not a number)
    char* test3_argv[] = {(char*)"./program", (char*)"one", (char*)"two", (char*)"three"};
    run_test_case("Test Case 3:", sizeof(test3_argv)/sizeof(char*), test3_argv); // Expected: Error

    // Test Case 4: Invalid index (zero)
    char* test4_argv[] = {(char*)"./program", (char*)"0", (char*)"a", (char*)"b"};
    run_test_case("Test Case 4:", sizeof(test4_argv)/sizeof(char*), test4_argv); // Expected: Error
    
    // Test Case 5: Invalid index (negative)
    char* test5_argv[] = {(char*)"./program", (char*)"-1", (char*)"a", (char*)"b"};
    run_test_case("Test Case 5:", sizeof(test5_argv)/sizeof(char*), test5_argv); // Expected: Error

    return result ? 0 : 1;
}