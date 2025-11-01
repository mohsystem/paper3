
#include <iostream>
#include <limits>
#include <string>
#include <cstdlib>

// Predefined array of ID sequences
const int id_sequence[] = {100, 200, 300, 400, 500};
const size_t id_sequence_length = sizeof(id_sequence) / sizeof(id_sequence[0]);

/**
 * Returns the value at the specified index in the id_sequence array.
 * Validates index bounds before access to prevent out-of-bounds errors.
 * 
 * @param index The index to retrieve
 * @param result Pointer to store the result (nullptr check performed)
 * @return true if successful, false if index is invalid or result is nullptr
 */
bool getIdAtIndex(int index, int* result) {
    // Pointer validation: ensure result pointer is not null
    if (result == nullptr) {
        return false;
    }
    
    // Input validation: check if index is within valid bounds
    // This prevents array out-of-bounds access (CWE-125)
    // Cast to size_t after confirming non-negative to avoid signed/unsigned comparison issues
    if (index < 0 || static_cast<size_t>(index) >= id_sequence_length) {
        // Fail closed: return error without exposing internal details
        return false;
    }
    
    // Safe array access after bounds validation
    *result = id_sequence[index];
    return true;
}

/**
 * Helper function to run test cases
 */
void testCase(int index, int expected, bool shouldSucceed) {
    int result = 0;
    bool success = getIdAtIndex(index, &result);
    
    bool passed = (success == shouldSucceed) && 
                  (!shouldSucceed || result == expected);
    
    std::cout << "Test: index=" << index 
              << ", expected=" << (shouldSucceed ? std::to_string(expected) : "FAIL")
              << ", got=" << (success ? std::to_string(result) : "FAIL")
              << " - " << (passed ? "PASS" : "FAIL") << std::endl;
}

int main() {
    try {
        // Run test cases
        std::cout << "Running test cases:" << std::endl;
        testCase(0, 100, true);
        testCase(2, 300, true);
        testCase(4, 500, true);
        testCase(-1, 0, false);   // Invalid: negative index
        testCase(10, 0, false);   // Invalid: index out of bounds
        
        std::cout << "\\nInteractive mode:" << std::endl;
        std::cout << "Enter an index value (0-" << (id_sequence_length - 1) << "): ";
        
        int index = 0;
        
        // Input validation: check if input is valid integer
        if (!(std::cin >> index)) {
            // Clear error flags and discard invalid input
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
            std::cout << "Error: Invalid input. Please enter a valid integer." << std::endl;
            return EXIT_FAILURE;
        }
        
        // Clear remaining input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
        
        int result = 0;
        
        // Call function with bounds checking
        if (getIdAtIndex(index, &result)) {
            std::cout << "Value at index " << index << ": " << result << std::endl;
        } else {
            // Generic error message - does not expose internal details
            std::cout << "Error: Invalid index." << std::endl;
            return EXIT_FAILURE;
        }
        
    } catch (const std::exception& e) {
        // Catch standard exceptions without exposing details
        std::cout << "Error: An unexpected error occurred." << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        // Catch-all for non-standard exceptions
        std::cout << "Error: An unexpected error occurred." << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
