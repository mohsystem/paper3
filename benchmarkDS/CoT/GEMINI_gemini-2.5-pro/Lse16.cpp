#include <iostream>
#include <vector>
#include <climits> // For INT_MIN

/**
 * Returns the value at the given index from a predefined array.
 *
 * @param index The integer index to look up.
 * @return The value at the specified index. If the index is out of bounds,
 *         it returns INT_MIN to signify an error.
 */
int getValueAtIndex(int index) {
    // A pre-defined constant vector. Using a static const vector is a safe way
    // to have a pre-initialized array-like structure.
    const static std::vector<int> PREDEFINED_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // Security Check: Ensure the index is non-negative and within the vector's bounds.
    // This prevents out-of-bounds access. We cast index to size_t for a safe comparison.
    if (index < 0 || static_cast<size_t>(index) >= PREDEFINED_ARRAY.size()) {
        // Return a special value to indicate an error.
        return INT_MIN;
    }
    
    // If the index is valid, return the element at that position.
    return PREDEFINED_ARRAY[index];
}

/**
 * Main function with 5 test cases to demonstrate the functionality.
 */
int main() {
    // Test cases including valid, edge, and invalid indices.
    int testCases[] = {5, 0, 9, -1, 10};

    std::cout << "Running C++ Test Cases..." << std::endl;
    for (int index : testCases) {
        std::cout << "Testing index: " << index << std::endl;
        int result = getValueAtIndex(index);

        // Check for the error value.
        if (result == INT_MIN) {
            std::cout << "Error: Index is out of bounds." << std::endl;
        } else {
            std::cout << "Value at index " << index << ": " << result << std::endl;
        }
        std::cout << "---" << std::endl;
    }
    
    return 0;
}