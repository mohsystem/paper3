#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * Securely gets the value at a specified index of a vector.
 *
 * @param vec The vector to access.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index.
 * @throws std::out_of_range if the index is out of bounds.
 */
int getValueAtIndex(const std::vector<int>& vec, int index) {
    // Secure: The .at() method automatically performs bounds checking and
    // throws std::out_of_range if the index is invalid.
    return vec.at(index);
}

void run_test(const std::vector<int>& data, int index) {
    try {
        int value = getValueAtIndex(data, index);
        std::cout << "Test case index: " << index << " -> Success! Value: " << value << std::endl;
    } catch (const std::out_of_range& e) {
        std::cout << "Test case index: " << index << " -> Caught expected exception: " << e.what() << std::endl;
    }
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    
    std::cout << "Array: [";
    for (size_t i = 0; i < data.size(); ++i) {
        std::cout << data[i] << (i == data.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;

    // Test Cases
    run_test(data, 5);  // Valid case
    run_test(data, 0);  // Valid edge case (first element)
    run_test(data, 9);  // Valid edge case (last element)
    run_test(data, -1); // Invalid case (negative index)
    run_test(data, 10); // Invalid case (index equal to size)

    return 0;
}