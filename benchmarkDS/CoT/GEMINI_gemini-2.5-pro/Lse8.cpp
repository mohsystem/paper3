#include <iostream>
#include <string>
#include <vector>

// Using a struct to return both the result and a status flag.
struct Result {
    std::string value;
    bool success;
};

/**
 * Retrieves the command-line argument at the index specified by the first argument.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of C-style string arguments.
 * @return A Result struct containing the value and a success flag.
 */
Result getValueAtIndex(int argc, char* argv[]) {
    // Security Check 1: Ensure there are enough arguments.
    // We need at least argv[1] (the index).
    if (argc < 2) {
        std::cerr << "Error: Not enough arguments. Usage: ./program <index> <arg1> ..." << std::endl;
        return {"", false};
    }

    int index;
    std::string index_str = argv[1];
    try {
        // Security Check 2: Safely parse the index from a string to an integer.
        // std::stoi is safer than atoi as it throws exceptions on failure.
        size_t processed_chars;
        index = std::stoi(index_str, &processed_chars);
        // Ensure the entire string was a valid number
        if (processed_chars != index_str.length()) {
            throw std::invalid_argument("Invalid characters in index string");
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: The first argument ('" << argv[1] << "') must be a valid integer." << std::endl;
        return {"", false};
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: The first argument ('" << argv[1] << "') is out of the integer range." << std::endl;
        return {"", false};
    }

    // Security Check 3: Perform bounds checking.
    if (index < 0 || index >= argc) {
        std::cerr << "Error: Index " << index << " is out of bounds. Valid range is [0, " << (argc - 1) << "]." << std::endl;
        return {"", false};
    }

    // If all checks pass, return the value.
    return {argv[index], true};
}

int main(int argc, char* argv[]) {
    Result result = getValueAtIndex(argc, argv);

    if (result.success) {
        std::cout << "Value at index " << argv[1] << ": " << result.value << std::endl;
        return 0; // Success
    } else {
        return 1; // Failure
    }

    // --- Test Cases ---
    // This program must be compiled and run from the command line.
    //
    // Compile first: g++ -Wall -std=c++11 your_file.cpp -o program
    //
    // Test Case 1: Successful retrieval
    // Command: ./program 3 apple banana orange grape
    // Expected Output: Value at index 3: orange
    //
    // Test Case 2: Index points to the program name itself (index 0)
    // Command: ./program 0 test one two
    // Expected Output: Value at index 0: ./program
    //
    // Test Case 3: Error - Index out of bounds
    // Command: ./program 5 apple banana
    // Expected Output: Error: Index 5 is out of bounds. Valid range is [0, 3].
    //
    // Test Case 4: Error - Invalid index format
    // Command: ./program two apple banana
    // Expected Output: Error: The first argument ('two') must be a valid integer.
    //
    // Test Case 5: Error - Not enough arguments
    // Command: ./program
    // Expected Output: Error: Not enough arguments. Usage: ./program <index> <arg1> ...
}