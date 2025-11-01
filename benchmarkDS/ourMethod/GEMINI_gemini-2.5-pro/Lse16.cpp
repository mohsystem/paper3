#include <iostream>
#include <vector>
#include <stdexcept>

// Predefined data vector
const std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * Returns the value at the specified index in a predefined vector.
 * Using vector::at for automatic bounds checking.
 *
 * @param index The index of the element to return.
 * @return The integer value at the specified index.
 * @throws std::out_of_range if the index is out of range.
 */
int getValueAtIndex(int index) {
    // Input validation is handled by vector::at, which throws std::out_of_range
    // if the index is not within the valid range [0, size()-1].
    // This is a secure way to access elements as it prevents out-of-bounds reads.
    return data.at(index);
}

int main() {
    // 5 Test Cases
    int testIndices[] = {0, 5, 9, -1, 10};
    int numTestCases = sizeof(testIndices) / sizeof(testIndices[0]);

    for (int i = 0; i < numTestCases; ++i) {
        int index = testIndices[i];
        std::cout << "Test Case " << (i + 1) << ": Trying to access index " << index << std::endl;
        try {
            int value = getValueAtIndex(index);
            std::cout << " -> Success: Value at index " << index << " is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << " -> Caught expected exception: " << e.what() << std::endl;
        }
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}