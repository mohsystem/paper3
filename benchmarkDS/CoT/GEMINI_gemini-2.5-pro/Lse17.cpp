#include <iostream>
#include <vector>
#include <limits>

/**
 * Returns the value at a specific index in a vector.
 * Performs bounds checking to prevent out-of-bounds access.
 *
 * @param arr The source vector of integers.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index, or a sentinel value (int_min) if the index is invalid.
 */
int getValueAtIndex(const std::vector<int>& arr, int index) {
    // Security: Validate the index to ensure it is within the valid bounds of the vector.
    // This prevents undefined behavior from out-of-bounds access, which is a major
    // security risk in C++.
    if (index < 0 || static_cast<size_t>(index) >= arr.size()) {
        std::cerr << "Error: Index " << index << " is out of bounds." << std::endl;
        return std::numeric_limits<int>::min(); // Return a sentinel value.
    }
    return arr[index];
}

int main() {
    std::vector<int> data = {10, 20, 30, 40, 50, 60, 70};
    const int INT_MIN_SENTINEL = std::numeric_limits<int>::min();

    // --- 5 Test Cases ---

    // Test Case 1: Valid index in the middle
    int index1 = 3;
    int value1 = getValueAtIndex(data, index1);
    if (value1 != INT_MIN_SENTINEL) {
        std::cout << "Test 1: Value at index " << index1 << " is " << value1 << std::endl;
    }

    // Test Case 2: Valid index at the beginning
    int index2 = 0;
    int value2 = getValueAtIndex(data, index2);
    if (value2 != INT_MIN_SENTINEL) {
        std::cout << "Test 2: Value at index " << index2 << " is " << value2 << std::endl;
    }

    // Test Case 3: Valid index at the end
    int index3 = data.size() - 1;
    int value3 = getValueAtIndex(data, index3);
    if (value3 != INT_MIN_SENTINEL) {
        std::cout << "Test 3: Value at index " << index3 << " is " << value3 << std::endl;
    }

    // Test Case 4: Invalid index (negative)
    int index4 = -1;
    std::cout << "Test 4: ";
    getValueAtIndex(data, index4);

    // Test Case 5: Invalid index (too large)
    int index5 = 10;
    std::cout << "Test 5: ";
    getValueAtIndex(data, index5);

    return 0;
}