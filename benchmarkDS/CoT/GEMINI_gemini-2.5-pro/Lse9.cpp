#include <iostream>
#include <string>
#include <vector>

/**
 * Takes command line arguments, parses the first argument as an index,
 * and prints the argument at that raw index.
 *
 * @param argc The argument count.
 * @param argv The argument vector.
 */
void printArgumentByIndex(int argc, char* argv[]) {
    // argv[0] is the program name. Need at least argv[1] (the index).
    if (argc < 2) {
        std::cerr << "Error: No index provided. Usage: ./program <index> [args...]" << std::endl;
        return;
    }

    int index;
    try {
        // std::stoi can throw an exception if conversion fails.
        index = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid index '" << argv[1] << "'. Index must be an integer." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Index '" << argv[1] << "' is out of range for an integer." << std::endl;
        return;
    }

    // The user-provided index N corresponds to argv[N].
    // We must check if this index is valid for the argv array.
    if (index >= 0 && index < argc) {
        std::cout << argv[index] << std::endl;
    } else {
        std::cerr << "Error: Index " << index << " is out of bounds for arguments array of size " << argc << "." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // This program processes actual command-line arguments.
    // The following comments show 5 test cases and how to run them from the terminal.

    /*
     * To compile and run:
     * g++ -std=c++11 -o program your_source.cpp
     *
     * --- Test Cases ---
     *
     * 1. Normal Case: Get the argument at index 3.
     *    Command: ./program 3 apple banana cherry
     *    Expected Output: cherry
     *    (argv is ['./program', '3', 'apple', 'banana', 'cherry'], argv[3] is 'cherry')
     *
     * 2. Edge Case (Program Name): Get the argument at index 0.
     *    Command: ./program 0 apple banana
     *    Expected Output: ./program
     *
     * 3. Error Case (Index Out of Bounds): Index is too high.
     *    Command: ./program 5 apple banana
     *    Expected Output: Error: Index 5 is out of bounds for arguments array of size 4.
     *
     * 4. Error Case (Invalid Index): Index is not a number.
     *    Command: ./program foo apple banana
     *    Expected Output: Error: Invalid index 'foo'. Index must be an integer.
     *
     * 5. Error Case (Index points to itself): Get the argument at index 1.
     *    Command: ./program 1 apple banana
     *    Expected Output: 1
     */

    printArgumentByIndex(argc, argv);
    return 0;
}