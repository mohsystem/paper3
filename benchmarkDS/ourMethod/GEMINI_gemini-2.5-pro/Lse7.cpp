#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Retrieves an argument from an array at an index specified by another argument.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return A std::string containing the argument at the specified index or an error message.
 */
std::string getArgumentAtIndex(int argc, const char* const argv[]) {
    // Rule #4: Validate input - ensure there are enough arguments
    // We need at least the program name and an index argument.
    if (argc < 2) {
        return "Error: Insufficient arguments. At least two are required.";
    }

    long index;
    try {
        std::string index_str = argv[1];
        size_t pos = 0;
        // Rule #5: Use safer conversion that detects errors
        index = std::stol(index_str, &pos);
        
        // Rule #4: Validate input format - ensure entire string was a number
        if (pos != index_str.length()) {
            return "Error: The second argument '" + index_str + "' contains non-numeric characters.";
        }
    } catch (const std::invalid_argument&) {
        return "Error: The second argument is not a valid integer.";
    } catch (const std::out_of_range&) {
        return "Error: The index value is out of range.";
    }

    // Rule #1, #2: Ensure the index is within the buffer's boundaries
    if (index < 0 || index >= argc) {
        return "Error: Index " + std::to_string(index) + " is out of bounds for the arguments array (size: " + std::to_string(argc) + ").";
    }

    return std::string(argv[index]);
}

void runTest(const std::string& testName, int argc, const char* const argv[]) {
    std::cout << "\n--- " << testName << " ---" << std::endl;
    std::cout << "Input: ";
    for (int i = 0; i < argc; ++i) {
        std::cout << "\"" << argv[i] << "\" ";
    }
    std::cout << std::endl;
    std::cout << "Output: " << getArgumentAtIndex(argc, argv) << std::endl;
}

int main(int argc, const char* const argv[]) {
    // Process command-line arguments if provided
    if (argc > 1) {
        std::cout << "--- Processing Command Line Arguments ---" << std::endl;
        runTest("Command Line Input", argc, argv);
        std::cout << "----------------------------------------" << std::endl;
    }
    
    std::cout << "\n--- Running Built-in Test Cases ---" << std::endl;

    // Test Case 1: Valid index
    const char* test1[] = {"./program", "2", "target_arg", "arg3"};
    runTest("Test Case 1: Valid index", 4, test1);

    // Test Case 2: Index pointing to the program name
    const char* test2[] = {"./program", "0", "arg2"};
    runTest("Test Case 2: Index points to program name", 3, test2);

    // Test Case 3: Index out of bounds (too high)
    const char* test3[] = {"./program", "3"};
    runTest("Test Case 3: Index out of bounds (high)", 2, test3);

    // Test Case 4: Invalid index format (not an integer)
    const char* test4[] = {"./program", "two", "arg2"};
    runTest("Test Case 4: Invalid index format", 3, test4);

    // Test Case 5: Insufficient arguments
    const char* test5[] = {"./program"};
    runTest("Test Case 5: Insufficient arguments", 1, test5);
    
    return 0;
}