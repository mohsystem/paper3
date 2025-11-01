#include <iostream>
#include <vector>
#include <cstddef>

/**
 * Returns the value from an array at a given index.
 *
 * @param arr The vector of integers.
 * @param size The size of the vector.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueFromArray(const std::vector<int>& arr, size_t size, int index) {
    // Ensure that buffer operations strictly adhere to the buffer's boundaries.
    // The cast to size_t is safe because we already checked index >= 0.
    if (index >= 0 && static_cast<size_t>(index) < size) {
        return arr[static_cast<size_t>(index)];
    }
    // Return -1 to indicate an out-of-bounds error.
    return -1;
}

int main() {
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
    const std::vector<int> id_sequence = {10, 20, 30, 40, 50};
    const size_t size = id_sequence.size();
    int index;

    // Ensure all input is validated.
    if (!(std::cin >> index)) {
        // Handle non-integer input or EOF
        if (std::cin.eof()) {
            std::cerr << "Error: End-of-file reached before input." << std::endl;
        } else {
            std::cerr << "Error: Invalid input. Please enter an integer." << std::endl;
        }
        return 1;
    }

    int value = getValueFromArray(id_sequence, size, index);

    return value;
}