#include <stdio.h>
#include <stddef.h> // For size_t

// The predefined sequence of IDs.
static const int id_sequence[] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};
// Calculate the number of elements in the array at compile time.
static const size_t id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Gets the value at the specified index in the id_sequence array.
 *
 * @param index The index of the element to retrieve.
 * @param result A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., index out of bounds or null result pointer).
 */
int getValueAtIndex(int index, int* result) {
    // Rule #6: Check for null pointers.
    if (result == NULL) {
        fprintf(stderr, "Error: result pointer cannot be NULL.\n");
        return -1;
    }
    
    // Rule #1 & #3: Validate input and check boundaries.
    // Note: 'index' is int, 'id_sequence_size' is size_t. The comparison is safe
    // as we first check if index is non-negative.
    if (index < 0 || (size_t)index >= id_sequence_size) {
        fprintf(stderr, "Error: Index %d is out of bounds. Valid range is [0, %zu].\n", index, id_sequence_size - 1);
        *result = 0; // Set result to a predictable value on error
        return -1;
    }

    *result = id_sequence[index];
    return 0;
}

// Helper function to run a single test case
void run_test(const char* test_name, int index, int* result_ptr) {
    printf("--- Test Case: %s ---\n", test_name);
    printf("Attempting to get value at index: %d", index);
    if (result_ptr == NULL) {
        printf(" (with NULL result pointer)\n");
    } else {
        printf("\n");
    }
    
    int value = 0;
    // Rule #6: Check all function return values
    if (getValueAtIndex(index, result_ptr) == 0) {
        if (result_ptr != NULL) {
           printf("Success! Value: %d\n", *result_ptr);
        }
    } else {
        printf("Failure as expected.\n");
    }
    printf("\n");
}

int main() {
    int result_value;

    // 5 test cases
    run_test("Valid Start Index", 0, &result_value);
    run_test("Valid End Index", 9, &result_value);
    run_test("Invalid Negative Index", -1, &result_value);
    run_test("Invalid Out-of-Bounds Index", 10, &result_value);
    run_test("Invalid NULL Pointer", 0, NULL);
    
    return 0;
}