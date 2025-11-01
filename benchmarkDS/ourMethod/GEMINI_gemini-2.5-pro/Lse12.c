#include <stdio.h>
#include <stdlib.h> // For EXIT_FAILURE, EXIT_SUCCESS

/**
 * @brief Returns the value at a given index in an array.
 *
 * This function performs bounds checking. If the index is out of bounds,
 * it prints an error message to stderr and returns EXIT_FAILURE.
 *
 * @param arr The array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index on success, or EXIT_FAILURE on error.
 */
int getValueFromArray(const int* arr, int size, int index) {
    // Rule #1, #4: Validate inputs and check boundaries
    if (arr == NULL) {
        fprintf(stderr, "Error: The provided array is NULL.\n");
        return EXIT_FAILURE;
    }
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index %d is out of bounds for array of size %d.\n", index, size);
        return EXIT_FAILURE;
    }
    return arr[index];
}

/**
 * @brief Helper function to run a single test case.
 */
void runTestCase(const int* arr, int size, int index, int expected) {
    printf("Test case: index=%d. ", index);
    int result = getValueFromArray(arr, size, index);
    if (result == expected) {
        printf("Result: %d. PASS\n", result);
    } else {
        printf("Result: %d. FAIL (Expected: %d)\n", result, expected);
    }
}

int main(void) {
    const int data[] = {10, 20, 30, 40, 50};
    const int dataSize = sizeof(data) / sizeof(data[0]);

    // --- Test Cases ---
    printf("--- Running Test Cases ---\n");
    runTestCase(data, dataSize, 0, 10);
    runTestCase(data, dataSize, 4, 50);
    runTestCase(data, dataSize, 2, 30);
    runTestCase(data, dataSize, -1, EXIT_FAILURE); // Expect failure
    runTestCase(data, dataSize, 5, EXIT_FAILURE);  // Expect failure
    printf("--- Test Cases Finished ---\n\n");

    printf("Enter an index to retrieve a value from the array:\n");
    int index;
    
    // Rule #4: Input validation, check scanf return value
    if (scanf("%d", &index) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter an integer.\n");
        return EXIT_FAILURE;
    }

    int result = getValueFromArray(data, dataSize, index);
    
    // Check if the call was successful before printing the result.
    // This handles the edge case where the value in the array is EXIT_FAILURE.
    int was_successful = !(result == EXIT_FAILURE && (index < 0 || index >= dataSize));
    if (was_successful) {
        printf("Value at index %d is %d\n", index, result);
    }

    // Use the return value from the function as the exit code
    return result;
}