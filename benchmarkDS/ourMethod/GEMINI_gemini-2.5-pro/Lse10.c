#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * Returns the value from an array at a given index.
 *
 * @param arr The array of integers.
 * @param size The size of the array.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const int* arr, size_t size, int index) {
    // Ensure that buffer operations strictly adhere to the buffer's boundaries.
    // The cast to size_t is safe because we already checked index >= 0.
    if (arr != NULL && index >= 0 && (size_t)index < size) {
        return arr[(size_t)index];
    }
    // Return -1 to indicate an out-of-bounds error or NULL array.
    return -1;
}

int main(void) {
    /*
    Test cases:
    Let id_sequence = {10, 20, 30, 40, 50}
    To test, compile and run, then provide input.
    On Linux/macOS: ./your_program; echo $?
    On Windows (cmd): your_program.exe & echo %errorlevel%
    
    1. Input: 0
       Expected exit code: 10
    2. Input: 4
       Expected exit code: 50
    3. Input: 5 (out of bounds)
       Expected exit code: 255 (-1 wrapped to unsigned 8-bit on POSIX)
    4. Input: -1 (out of bounds)
       Expected exit code: 255
    5. Input: "abc" (non-integer)
       Expected exit code: 1
    */
    const int id_sequence[] = {10, 20, 30, 40, 50};
    const size_t size = sizeof(id_sequence) / sizeof(id_sequence[0]);
    int index = 0;

    // Ensure all input is validated.
    if (scanf("%d", &index) != 1) {
        fprintf(stderr, "Error: Invalid input. Please enter an integer.\n");
        return 1;
    }

    int value = getValueFromArray(id_sequence, size, index);

    return value;
}