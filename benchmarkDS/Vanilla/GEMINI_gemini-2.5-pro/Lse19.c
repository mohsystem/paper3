#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Adds 1000 to the given integer.
 * @param number The input integer.
 * @return The input number plus 1000.
 */
int addThousand(int number) {
    return number + 1000;
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, process it.
    if (argc > 1) {
        // atoi returns 0 on error, which is acceptable for this simple case.
        // For more robust error handling, strtol should be used.
        int inputNumber = atoi(argv[1]);
        int result = addThousand(inputNumber);
        printf("%d\n", result);
    } else {
        // Otherwise, run the 5 built-in test cases.
        printf("No command-line argument provided. Running test cases:\n");
        int testCases[] = {10, -50, 0, 12345, -1000};
        int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
        for (int i = 0; i < numTestCases; i++) {
            int testInput = testCases[i];
            int result = addThousand(testInput);
            printf("Input: %d, Output: %d\n", testInput, result);
        }
    }
    return 0;
}