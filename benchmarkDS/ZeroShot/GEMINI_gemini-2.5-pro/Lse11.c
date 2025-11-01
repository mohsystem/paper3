#include <stdio.h>
#include <stdlib.h> // For exit()

// A predefined array
const int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
const int id_sequence_size = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * @brief Securely gets a value from an array at a given index.
 * 
 * Performs bounds checking to prevent out-of-bounds access.
 * If the index is out of bounds, it prints an error and terminates the program.
 * 
 * @param array The array to access.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 */
int getValueFromArray(const int* array, int size, int index) {
    // Securely check if the index is within the valid bounds of the array.
    if (index < 0 || index >= size) {
        fprintf(stderr, "Error: Index out of bounds.\n");
        // Exit with a non-zero status code to indicate an error.
        exit(1);
    }
    return array[index];
}

/**
 * @brief The main function reads an integer index from standard input, retrieves a value
 * from a predefined array at that index, and returns the value as the exit code.
 */
int main() {
    /*
     * Test cases to run manually by providing input:
     * 1. Input: 3  (valid index) 
     *    Expected: Program exits with code 40.
     * 2. Input: 0  (valid first element)
     *    Expected: Program exits with code 10.
     * 3. Input: 9  (valid last element)
     *    Expected: Program exits with code 100.
     * 4. Input: -1 (invalid, negative)
     *    Expected: "Error: Index out of bounds." on stderr, program exits with code 1.
     * 5. Input: 10 (invalid, too large)
     *    Expected: "Error: Index out of bounds." on stderr, program exits with code 1.
     */
    int index;

    // Securely check if scanf successfully read one integer.
    if (scanf("%d", &index) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter an integer.\n");
        return 1; // Return 1 to indicate error
    }

    int value = getValueFromArray(id_sequence, id_sequence_size, index);
    
    // The return value of main is the program's exit code.
    // Note: Exit codes are typically 0-255. Larger values may be truncated.
    return value;
}