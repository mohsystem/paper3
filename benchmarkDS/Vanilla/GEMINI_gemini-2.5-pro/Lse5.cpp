#include <iostream>
#include <vector>
#include <string>

/**
 * Accepts a vector and an index, then prints the element at that index.
 * It includes error handling for out-of-bounds indices.
 * @param data The vector to access.
 * @param index The index of the element to print.
 */
void getElementAtIndex(const std::vector<int>& data, int index) {
    if (index >= 0 && index < data.size()) {
        std::cout << "Element at index " << index << ": " << data[index] << std::endl;
    } else {
        std::cout << "Error: Index " << index << " is out of bounds for a vector of size " << data.size() << "." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::vector<int> elements = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    // Check if a command-line argument is provided
    if (argc > 1) {
        try {
            int indexFromArg = std::stoi(argv[1]);
            std::cout << "--- Running with command-line argument ---" << std::endl;
            getElementAtIndex(elements, indexFromArg);
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid command-line argument. Please provide an integer." << std::endl;
        }
    } else {
        // If no command-line arguments, run the 5 built-in test cases
        std::cout << "--- Running built-in test cases ---" << std::endl;
        
        // Test Case 1: Accessing the first element
        getElementAtIndex(elements, 0);
        
        // Test Case 2: Accessing an element in the middle
        getElementAtIndex(elements, 4);

        // Test Case 3: Accessing the last element
        getElementAtIndex(elements, 9);
        
        // Test Case 4: Accessing with a negative index (out of bounds)
        getElementAtIndex(elements, -1);
        
        // Test Case 5: Accessing an index equal to the vector size (out of bounds)
        getElementAtIndex(elements, 10);
    }

    return 0;
}