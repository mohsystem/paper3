#include <iostream>
#include <vector>
#include <stdexcept> // For std::out_of_range
#include <string>    // For std::to_string

/**
 * @brief Returns the value at a specified index in a given vector.
 * This function performs bounds checking to prevent Out-of-bounds Read vulnerabilities.
 *
 * @param data The constant reference to the vector to access.
 * @param index The index of the element to retrieve.
 * @return The integer value at the specified index.
 * @throw std::out_of_range if the index is out of the vector's bounds.
 */
int getValueAtIndex(const std::vector<int>& data, int index) {
    // Rule #1, #4: Validate input index to ensure it is within the buffer's boundaries.
    // The check `index < 0` handles negative indices.
    // The cast to size_t handles the positive boundary check safely. If index is
    // negative, its unsigned representation will be a very large number,
    // which will correctly fail the `< data.size()` check.
    if (index < 0 || static_cast<size_t>(index) >= data.size()) {
        throw std::out_of_range("Index " + std::to_string(index) + " is out of bounds for vector of size " + std::to_string(data.size()));
    }
    // Access is safe after the bounds check.
    return data[static_cast<size_t>(index)];
}

int main() {
    // The vector from which to retrieve a value. std::vector is preferred over C-style arrays.
    const std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // 5 test cases, including valid, edge, and invalid indices.
    int testIndices[] = {0, 5, 9, -1, 10};

    std::cout << "Running test cases for vector: { ";
    for(size_t i = 0; i < data.size(); ++i) {
        std::cout << data[i] << (i == data.size() - 1 ? "" : ", ");
    }
    std::cout << " }" << std::endl;

    for (int i = 0; i < 5; ++i) {
        int index = testIndices[i];
        std::cout << "\n--- Test Case " << (i + 1) << ": Attempting to access index " << index << " ---" << std::endl;
        try {
            // Rule #7: Handle exceptions appropriately.
            int value = getValueAtIndex(data, index);
            std::cout << "Success: Value at index " << index << " is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            // Rule #8: Provide clear, minimal error messages.
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    return 0;
}