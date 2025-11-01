#include <iostream>
#include <string>
#include <stdexcept>

/**
 * Retrieves a command-line argument by a 1-based index.
 *
 * @param argc The argument count.
 * @param argv The argument vector. argv[1] is the index, argv[2...] are data.
 * @return The argument at the specified index.
 * @throws std::invalid_argument if arguments are invalid.
 */
std::string getArgumentByIndex(int argc, char* argv[]) {
    // 1. Security Check: Ensure enough arguments
    // We need at least 3: program_name, index, data_arg1
    if (argc < 3) {
        throw std::invalid_argument("Usage: ./Lse9 <index> <arg1> [arg2] ...");
    }

    int index;
    try {
        // 2. Security Check: Parse index, handle non-integer and out-of-range input
        std::string index_str(argv[1]);
        size_t pos;
        index = std::stoi(index_str, &pos);
        // Ensure the entire string was consumed, preventing inputs like "123xyz"
        if (pos != index_str.length()) {
             throw std::invalid_argument("Error: Index must be a valid integer.");
        }
    } catch (const std::logic_error& e) { // Catches std::invalid_argument and std::out_of_range
        throw std::invalid_argument("Error: Index must be a valid integer and within integer limits.");
    }

    // 3. Security Check: Ensure index is positive
    if (index < 1) {
        throw std::invalid_argument("Error: Index must be a positive integer (1-based).");
    }

    // Number of data arguments available
    int dataArgCount = argc - 2;

    // 4. Security Check: Ensure index is within bounds
    if (index > dataArgCount) {
        throw std::invalid_argument("Error: Index " + std::to_string(index) + " is out of bounds. Only " + std::to_string(dataArgCount) + " data arguments available.");
    }

    // The user's 1-based index `index` corresponds to argv[index + 1]
    // because argv[0] is program name and argv[1] is the index itself.
    return std::string(argv[index + 1]);
}

int main(int argc, char* argv[]) {
    // Note: These test cases are for demonstration.
    // You must run this program from the command line to provide arguments.

    /*
    To compile and run from the command line:
    g++ -std=c++11 -o Lse9 Lse9.cpp

    Test Cases:
    1. Valid case: Get the 2nd data argument
       Command: ./Lse9 2 apple banana orange
       Expected Output: banana

    2. Valid case: Get the 1st data argument
       Command: ./Lse9 1 apple banana orange
       Expected Output: apple

    3. Error case: Index out of bounds
       Command: ./Lse9 4 apple banana orange
       Expected Output: Error: Index 4 is out of bounds. Only 3 data arguments available.

    4. Error case: Non-integer index
       Command: ./Lse9 two apple banana
       Expected Output: Error: Index must be a valid integer and within integer limits.

    5. Error case: Not enough arguments
       Command: ./Lse9 1
       Expected Output: Usage: ./Lse9 <index> <arg1> [arg2] ...
    */
    
    try {
        std::string argument = getArgumentByIndex(argc, argv);
        std::cout << argument << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
        return 1; // Indicate error
    }

    return 0; // Indicate success
}