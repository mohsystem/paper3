#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Converts the string at index 1 of the argv array to an integer 'INDEX',
 * then prints the argument at position 'INDEX'.
 *
 * @param argc The argument count.
 * @param argv An array of C-style strings (char*).
 */
void printArgFromIndex(int argc, const char* argv[]) {
    // Check if there are enough arguments to proceed. We need at least 2.
    if (argc <= 1) {
        std::cout << "Error: Not enough arguments. Please provide an index." << std::endl;
        return;
    }

    int index;
    try {
        // Convert the second argument (at index 1) to an integer.
        index = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cout << "Error: The second argument must be a valid integer." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cout << "Error: The index value is out of range for an integer." << std::endl;
        return;
    }

    // Check if the parsed index is within the valid bounds of the array.
    if (index < 0 || index >= argc) {
        std::cout << "Error: Index " << index << " is out of bounds for the given arguments." << std::endl;
        return;
    }

    // Print the argument at the specified index.
    std::cout << argv[index] << std::endl;
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1: Valid case, should print "banana"
    std::cout << "--- Test Case 1 ---" << std::endl;
    std::cout << "Input: {\"./Lse7\", \"3\", \"apple\", \"banana\", \"cherry\"}" << std::endl;
    std::cout << "Output: ";
    const char* test1_argv[] = {"./Lse7", "3", "apple", "banana", "cherry"};
    int test1_argc = sizeof(test1_argv) / sizeof(char*);
    printArgFromIndex(test1_argc, test1_argv);
    std::cout << std::endl;

    // Test Case 2: Valid case, index 0, should print the program name
    std::cout << "--- Test Case 2 ---" << std::endl;
    std::cout << "Input: {\"./Lse7\", \"0\"}" << std::endl;
    std::cout << "Output: ";
    const char* test2_argv[] = {"./Lse7", "0"};
    int test2_argc = sizeof(test2_argv) / sizeof(char*);
    printArgFromIndex(test2_argc, test2_argv);
    std::cout << std::endl;

    // Test Case 3: Error case, index out of bounds
    std::cout << "--- Test Case 3 ---" << std::endl;
    std::cout << "Input: {\"./Lse7\", \"5\", \"a\", \"b\"}" << std::endl;
    std::cout << "Output: ";
    const char* test3_argv[] = {"./Lse7", "5", "a", "b"};
    int test3_argc = sizeof(test3_argv) / sizeof(char*);
    printArgFromIndex(test3_argc, test3_argv);
    std::cout << std::endl;

    // Test Case 4: Error case, non-integer index
    std::cout << "--- Test Case 4 ---" << std::endl;
    std::cout << "Input: {\"./Lse7\", \"two\", \"a\", \"b\"}" << std::endl;
    std::cout << "Output: ";
    const char* test4_argv[] = {"./Lse7", "two", "a", "b"};
    int test4_argc = sizeof(test4_argv) / sizeof(char*);
    printArgFromIndex(test4_argc, test4_argv);
    std::cout << std::endl;

    // Test Case 5: Error case, not enough arguments
    std::cout << "--- Test Case 5 ---" << std::endl;
    std::cout << "Input: {\"./Lse7\"}" << std::endl;
    std::cout << "Output: ";
    const char* test5_argv[] = {"./Lse7"};
    int test5_argc = sizeof(test5_argv) / sizeof(char*);
    printArgFromIndex(test5_argc, test5_argv);
    std::cout << std::endl;

    return 0;
}