#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// Predefined data array
const std::vector<int> data = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

/**
 * @brief Securely accesses an element in a predefined vector based on a string index.
 * Handles parsing errors and bounds checking.
 * @param indexStr The string representation of the index to access.
 */
void processIndex(const std::string& indexStr) {
    try {
        size_t pos;
        int index = std::stoi(indexStr, &pos);

        // Check if the entire string was consumed, ensuring no trailing characters
        if (pos != indexStr.length()) {
            std::cerr << "Error: Invalid input '" << indexStr << "'. Not a valid integer." << std::endl;
            return;
        }

        // Secure bounds checking
        if (index >= 0 && static_cast<size_t>(index) < data.size()) {
            std::cout << "Element at index " << index << ": " << data[index] << std::endl;
        } else {
            std::cerr << "Error: Index " << index << " is out of bounds." << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid input. Not an integer." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Input is out of the range of representable integer values." << std::endl;
    }
}

/**
 * @brief Runs a series of predefined test cases.
 */
void runTests() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid index
    std::cout << "Test 1 (Input: \"3\"): ";
    processIndex("3");

    // Test Case 2: Negative index (out of bounds)
    std::cout << "Test 2 (Input: \"-1\"): ";
    processIndex("-1");

    // Test Case 3: Index equal to vector size (out of bounds)
    std::cout << "Test 3 (Input: \"10\"): ";
    processIndex("10");

    // Test Case 4: Last valid index
    std::cout << "Test 4 (Input: \"9\"): ";
    processIndex("9");

    // Test Case 5: Non-numeric input
    std::cout << "Test 5 (Input: \"abc\"): ";
    processIndex("abc");
    
    std::cout << "--- Test Cases Finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        // Process the command line argument
        processIndex(argv[1]);
    } else {
        // Run built-in test cases if no/wrong number of arguments is provided
        std::cout << "Usage: " << argv[0] << " <index>" << std::endl;
        std::cout << "No command line argument provided. Running default test cases." << std::endl;
        runTests();
    }
    return 0;
}