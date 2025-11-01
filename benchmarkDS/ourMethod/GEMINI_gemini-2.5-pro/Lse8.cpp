#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Retrieves the argument at the index specified by argv[1].
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of C-style strings representing the arguments.
 * @return The argument at the specified index.
 * @throws std::invalid_argument if there are not enough arguments, the index is
 *         not a valid integer, or the index is out of bounds.
 */
std::string getArgumentByIndex(int argc, char* argv[]) {
    // Rule #4: Validate input - check if enough arguments are provided.
    if (argc < 2) {
        throw std::invalid_argument("Error: Not enough arguments. Please provide an index.");
    }

    int index;
    try {
        // Rule #4: Validate input type.
        // Rule #5: Use safer C++ style conversion that throws on error.
        std::string index_str(argv[1]);
        size_t pos;
        index = std::stoi(index_str, &pos);
        // Ensure the entire string was consumed
        if (pos != index_str.length()) {
            throw std::invalid_argument("Error: Index contains non-numeric characters.");
        }
    } catch (const std::invalid_argument&) {
        // Rule #7: Catch exceptions for invalid formats.
        throw std::invalid_argument("Error: The first argument must be a valid integer index.");
    } catch (const std::out_of_range&) {
        throw std::invalid_argument("Error: The provided index is out of the integer range.");
    }

    // Rule #1 & #4: Ensure the index is within the buffer's boundaries.
    if (index < 0 || index >= argc) {
        throw std::out_of_range("Error: Index " + std::to_string(index) + " is out of bounds.");
    }

    return std::string(argv[index]);
}

void runTest(const std::string& testName, int argc, char* argv[]) {
    std::cout << "\n" << testName << std::endl;
    std::cout << "Input: ";
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;

    try {
        std::string result = getArgumentByIndex(argc, argv);
        std::cout << "Output: " << result << std::endl;
    } catch (const std::exception& e) {
        // Rule #7: Properly handle exceptions
        std::cerr << e.what() << std::endl;
        std::cout << "Output: null" << std::endl;
    }
}

void runTests() {
    std::cout << "\n--- Running 5 test cases ---" << std::endl;

    // Test Case 1: Valid case
    char* test1[] = {(char*)"program", (char*)"2", (char*)"first", (char*)"second"};
    runTest("Test Case 1: Valid index", 4, test1);

    // Test Case 2: Index out of bounds (too high)
    char* test2[] = {(char*)"program", (char*)"5", (char*)"only", (char*)"three", (char*)"args"};
    runTest("Test Case 2: Index out of bounds (high)", 5, test2);

    // Test Case 3: Negative index
    char* test3[] = {(char*)"program", (char*)"-1", (char*)"a", (char*)"b"};
    runTest("Test Case 3: Negative index", 4, test3);
    
    // Test Case 4: Invalid index format (not a number)
    char* test4[] = {(char*)"program", (char*)"foo", (char*)"bar"};
    runTest("Test Case 4: Invalid index format", 3, test4);

    // Test Case 5: Not enough arguments
    char* test5[] = {(char*)"program"};
    runTest("Test Case 5: Not enough arguments", 1, test5);

    std::cout << "\n--- Tests finished ---" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "--- Running with actual command line arguments ---" << std::endl;
    try {
        std::string result = getArgumentByIndex(argc, argv);
        std::cout << "Value at specified index: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        // Rule #8: Fail closed
        return 1;
    }
    std::cout << "------------------------------------------------" << std::endl;

    runTests();
    
    return 0;
}