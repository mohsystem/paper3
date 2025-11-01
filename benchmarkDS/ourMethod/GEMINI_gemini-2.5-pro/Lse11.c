#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Gets the value from the array at the given index safely.
 * 
 * @param arr Pointer to the integer array.
 * @param size The number of elements in the array.
 * @param index The index of the element to retrieve.
 * @param value_out Pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., out of bounds, null pointers).
 */
int getValueFromArray(const int* arr, size_t size, int index, int* value_out) {
    // Rule #8: Check for null pointers.
    if (arr == NULL || value_out == NULL) {
        return -1;
    }
    // Rule #1: Ensure that all buffer operations strictly adhere to the buffer's boundaries.
    if (index < 0 || (size_t)index >= size) {
        return -1; // Error: index out of bounds
    }
    *value_out = arr[index];
    return 0; // Success
}

/**
 * @brief Runs a single test case and prints the result.
 */
void runTest(const int* arr, size_t size, int index, int expectedExitCode) {
    printf("Test with index %d: ", index);
    int value;
    int exitCode;

    if (getValueFromArray(arr, size, index, &value) == 0) {
        exitCode = value;
    } else {
        exitCode = 1; // Simulate error exit code
    }

    if (exitCode == expectedExitCode) {
        printf("PASSED (Expected exit code: %d, Got: %d)\n", expectedExitCode, exitCode);
    } else {
        printf("FAILED (Expected exit code: %d, Got: %d)\n", expectedExitCode, exitCode);
    }
}

/**
 * @brief Main function to read an index and return an array value as the exit code.
 */
int main(void) {
    const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    
    // --- Test Cases ---
    printf("--- Running Test Cases ---\n");
    runTest(id_sequence, size, 0, 10);    // Test case 1: Valid, first element
    runTest(id_sequence, size, 9, 100);   // Test case 2: Valid, last element
    runTest(id_sequence, size, -1, 1);    // Test case 3: Invalid, negative index
    runTest(id_sequence, size, 10, 1);    // Test case 4: Invalid, out of bounds
    // Test case 5 (invalid format) is handled by the interactive part
    printf("--- Test Cases Finished ---\n");

    printf("\nEnter an integer index:\n");
    
    int index;
    // Rule #4: Ensure all input is validated by checking scanf's return value.
    if (scanf("%d", &index) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter an integer.\n");
        return 1;
    }
    
    int value;
    // Rule #8: Check function return values.
    if (getValueFromArray(id_sequence, size, index, &value) == 0) {
        printf("Value at index %d is %d. Exiting with this value.\n", index, value);
        return value;
    } else {
        fprintf(stderr, "Error: Index out of bounds.\n");
        return 1;
    }
}