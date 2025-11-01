#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Adds 1000 to the given number.
 * 
 * @param number The integer to which 1000 will be added.
 * @return The sum of the number and 1000.
 */
int addThousand(int number) {
    return number + 1000;
}

/**
 * @brief Main function. If a command-line argument is provided, it adds 1000 to it and prints the result.
 * Otherwise, it runs 5 hardcoded test cases.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char *argv[]) {
    if (argc > 1) {
        char *endptr;
        errno = 0; // To distinguish success/failure after call
        long inputVal = strtol(argv[1], &endptr, 10);

        // Check for conversion errors
        if (endptr == argv[1] || *endptr != '\0' || errno == ERANGE) {
            fprintf(stderr, "Error: The provided argument is not a valid integer.\n");
            return 1;
        }
        
        int result = addThousand((int)inputVal);
        printf("%d\n", result);

    } else {
        printf("No command line argument provided. Running test cases:\n");
        // 5 test cases
        int testValues[] = {5, -10, 0, 999, -2000};
        int numTests = sizeof(testValues) / sizeof(testValues[0]);
        for (int i = 0; i < numTests; i++) {
            int val = testValues[i];
            int result = addThousand(val);
            printf("Input: %d, Output: %d\n", val, result);
        }
    }
    return 0;
}