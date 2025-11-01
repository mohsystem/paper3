#include <stdio.h>
#include <stdbool.h> // For bool type
#include <stdlib.h>  // For size_t

// Define a constant sequence of IDs.
static const int ID_SEQUENCE[] = {101, 102, 103, 104, 105, 106, 107, 108, 109, 110};

/**
 * Retrieves the value at a given index from the ID_SEQUENCE array.
 * This function demonstrates secure C practices by returning a status code
 * and using an output parameter for the result. It performs manual bounds
 * checking.
 *
 * @param index The index of the value to retrieve.
 * @param result A pointer to an integer where the result will be stored.
 * @return true if the index is valid and the value was retrieved, false otherwise.
 */
bool getValueAtIndex(int index, int* result) {
    // Check for null pointer to prevent crashes.
    if (result == NULL) {
        return false;
    }
    
    // In C, the size of a static array must be calculated manually.
    const size_t size = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);

    // Rules #1, #2, #4: Ensure the index is within the array's boundaries.
    // The check for index < 0 is crucial before comparing with unsigned size_t.
    if (index < 0 || (size_t)index >= size) {
        return false;
    }

    *result = ID_SEQUENCE[index];
    return true;
}

/**
 * Helper function to run a single test case and print the result.
 * @param testName The name of the test case.
 * @param index The index to test.
 */
void runTestCase(const char* testName, int index) {
    printf("%s with index %d: ", testName, index);
    int value;
    if (getValueAtIndex(index, &value)) {
        printf("Success! Value is %d\n", value);
    } else {
        fprintf(stderr, "Error: Index is out of bounds.\n");
    }
}

/**
 * Clears the standard input buffer.
 */
void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    const size_t sequence_size = sizeof(ID_SEQUENCE) / sizeof(ID_SEQUENCE[0]);
    runTestCase("Test Case 1", 0);
    runTestCase("Test Case 2", 5);
    runTestCase("Test Case 3", (int)sequence_size - 1);
    runTestCase("Test Case 4", -1);
    runTestCase("Test Case 5", (int)sequence_size);

    // The prompt "The function first prompts the user for an index value" is interpreted
    // as the main application logic prompting the user before calling the function.
    printf("\n--- Interactive User Test ---\n");
    printf("Please enter an index value: ");
    
    int userIndex;
    // Rule #4: Always check the return value of scanf to ensure input was successful.
    if (scanf("%d", &userIndex) == 1) {
        int value;
        if (getValueAtIndex(userIndex, &value)) {
            printf("The value at index %d is: %d\n", userIndex, value);
        } else {
            fprintf(stderr, "Error: Index %d is out of bounds.\n", userIndex);
        }
    } else {
        fprintf(stderr, "Error: Invalid input. Please enter a valid integer.\n");
        // Clear the buffer to prevent subsequent input operations from failing.
        clear_stdin();
    }

    return 0;
}