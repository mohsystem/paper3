#include <iostream>
#include <vector>

/**
 * Returns the value at a given index in an integer vector.
 * @param arr The input vector.
 * @param index The index of the element to retrieve.
 * @return The value at the specified index, or -1 if the index is out of bounds.
 */
int getValueAtIndex(const std::vector<int>& arr, int index) {
    if (index >= 0 && index < arr.size()) {
        return arr[index];
    } else {
        // Return a sentinel value to indicate the index is out of bounds
        return -1;
    }
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // 5 test cases
    std::cout << "Test Case 1 (index 0): " << getValueAtIndex(data, 0) << std::endl;
    std::cout << "Test Case 2 (index 5): " << getValueAtIndex(data, 5) << std::endl;
    std::cout << "Test Case 3 (index 9): " << getValueAtIndex(data, 9) << std::endl;
    std::cout << "Test Case 4 (index -1): " << getValueAtIndex(data, -1) << std::endl;
    std::cout << "Test Case 5 (index 10): " << getValueAtIndex(data, 10) << std::endl;

    return 0;
}