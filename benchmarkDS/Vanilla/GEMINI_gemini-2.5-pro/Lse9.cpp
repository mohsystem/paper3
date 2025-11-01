#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Prints a command-line argument based on a given index.
 * The index is provided as the first argument after the program name,
 * and it refers to the subsequent arguments.
 * @param argc The argument count.
 * @param argv The argument vector.
 */
void printArgByIndex(int argc, char* argv[]) {
    // We need at least 3 arguments: program name, index, and one argument value.
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <index> <arg1> <arg2> ..." << std::endl;
        return;
    }

    try {
        // The first user argument (argv[1]) is the user-provided index.
        int userIndex = std::stoi(argv[1]);

        if (userIndex <= 0) {
            std::cerr << "Error: Index must be a positive integer." << std::endl;
            return;
        }
        
        // The target argument is at index `userIndex + 1` because
        // argv[0] is the program name and argv[1] is the index itself.
        int targetIndex = userIndex + 1;

        if (argc > targetIndex) {
            std::cout << argv[targetIndex] << std::endl;
        } else {
            std::cerr << "Error: Index " << userIndex << " is out of bounds for the provided arguments." << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: The first argument must be a valid integer index." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: The index number is too large." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    /*
     * To run the test cases, compile the code and execute from the command line:
     *
     * g++ -o Lse9 Lse9.cpp
     *
     * Test Case 1: Get the 1st argument after the index
     * Command: ./Lse9 1 apple banana cherry
     * Expected Output: apple
     *
     * Test Case 2: Get the 3rd argument after the index
     * Command: ./Lse9 3 apple banana cherry
     * Expected Output: cherry
     *
     * Test Case 3: Index out of bounds
     * Command: ./Lse9 4 apple banana cherry
     * Expected Output: Error: Index 4 is out of bounds for the provided arguments.
     *
     * Test Case 4: Not enough arguments (no argument to select)
     * Command: ./Lse9 1
     * Expected Output: Usage: ./Lse9 <index> <arg1> <arg2> ...
     *
     * Test Case 5: Invalid index (not an integer)
     * Command: ./Lse9 hello world
     * Expected Output: Error: The first argument must be a valid integer index.
     */
    printArgByIndex(argc, argv);
    return 0;
}