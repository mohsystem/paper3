#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Use long long for a wider range of values
const long long OFFSET = 1000;

/**
 * Adds a constant offset to the given value.
 *
 * @param value The input number.
 * @return The value with the offset added.
 */
long long addOffset(long long value) {
    return value + OFFSET;
}

void process_input(const char* input_str) {
    char* endptr;
    errno = 0; // Reset errno before the call

    long long input_value = strtoll(input_str, &endptr, 10);

    // Check for conversion errors
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Input '%s' is out of range for a long long.\n", input_str);
    } else if (endptr == input_str) {
        fprintf(stderr, "Error: Input '%s' is not a valid number (no digits found).\n", input_str);
    } else if (*endptr != '\0') {
        fprintf(stderr, "Error: Input '%s' contains non-numeric characters.\n", input_str);
    } else {
        // No errors, proceed with calculation
        long long result = addOffset(input_value);
        printf("Input: %lld, Result: %lld\n", input_value, result);
    }
}

int main(int argc, char* argv[]) {
    // We can use the first command-line argument if provided.
    if (argc > 1) {
        printf("Processing command-line argument: %s\n", argv[1]);
        process_input(argv[1]);
        printf("\n--- Running Internal Test Cases ---\n");
    }

    // 5 internal test cases for demonstration
    const char* test_cases[] = {
        "123",          // Basic positive number
        "-1234",        // Basic negative number
        "0",            // Zero
        "9223372036854775807", // LLONG_MAX, will cause overflow on parsing
        "1a2b"          // Invalid input
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Processing test case: %s\n", test_cases[i]);
        process_input(test_cases[i]);
    }

    return 0;
}