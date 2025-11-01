#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * Gets the value from a vector at a specified index.
 *
 * @param arr The source vector of integers.
 * @param size The size of the vector.
 * @param index The index of the element to retrieve.
 * @return The integer value at the given index.
 * @throws std::out_of_range if the index is out of the valid range [0, size-1].
 */
int getValueFromArray(const std::vector<int>& arr, int size, int index) {
    // Security check: Ensure the index is within the valid bounds of the array.
    // The cast to size_t is safe because we already checked for negative index.
    if (index < 0 || static_cast<size_t>(index) >= size) {
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds for array size " + std::to_string(size));
    }
    return arr[index];
}

int main() {
    std::vector<int> testArray = {10, 20, 30, 40, 50};
    int size = testArray.size();

    // Test cases
    int testIndexes[] = {2, 0, 4, -1, 5};

    std::cout << "Running CPP Test Cases..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        int indexToTest = testIndexes[i];
        std::cout << "Test Case " << (i + 1) << ": Trying to get value at index " << indexToTest << std::endl;
        try {
            int value = getValueFromArray(testArray, size, indexToTest);
            std::cout << "  Success: Value is " << value << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "  Caught expected exception: " << e.what() << std::endl;
        }
    }

    return 0;
}