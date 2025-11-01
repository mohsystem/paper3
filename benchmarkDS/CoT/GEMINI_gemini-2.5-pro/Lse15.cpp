#include <iostream>
#include <vector>

// The predefined vector (using std::vector for safety and convenience)
const std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Takes an index and returns the value at that index in the id_sequence vector.
 * Performs bounds checking to prevent security vulnerabilities.
 * 
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(int index) {
    // Security check: ensure the index is within the valid bounds of the vector.
    // Using .at() would throw an exception, but we will check manually to return -1.
    if (index >= 0 && index < id_sequence.size()) {
        return id_sequence[index];
    } else {
        // Return a sentinel value to indicate an error (index out of bounds).
        return -1;
    }
}

int main() {
    // 5 test cases
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid index in the middle
    int index1 = 3;
    std::cout << "Test Case 1: Index = " << index1 << ", Value = " << getValueAtIndex(index1) << std::endl;

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    std::cout << "Test Case 2: Index = " << index2 << ", Value = " << getValueAtIndex(index2) << std::endl;

    // Test Case 3: Valid index at the end
    int index3 = id_sequence.size() - 1;
    std::cout << "Test Case 3: Index = " << index3 << ", Value = " << getValueAtIndex(index3) << std::endl;

    // Test Case 4: Invalid index (negative)
    int index4 = -1;
    std::cout << "Test Case 4: Index = " << index4 << ", Value = " << getValueAtIndex(index4) << std::endl;

    // Test Case 5: Invalid index (out of bounds)
    int index5 = id_sequence.size();
    std::cout << "Test Case 5: Index = " << index5 << ", Value = " << getValueAtIndex(index5) << std::endl;

    return 0;
}