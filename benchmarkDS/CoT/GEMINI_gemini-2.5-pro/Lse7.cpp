#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Converts a string to an integer.
 * @param s The string to convert.
 * @param out_val A reference to store the converted integer.
 * @return True if conversion is successful, false otherwise.
 */
bool convertStringToInt(const std::string& s, int& out_val) {
    try {
        size_t pos;
        out_val = std::stoi(s, &pos);
        // Ensure the entire string was consumed
        return pos == s.length();
    } catch (const std::invalid_argument& ia) {
        return false;
    } catch (const std::out_of_range& oor) {
        return false;
    }
}

/**
 * Prints the command line argument at the index specified by the second argument.
 * @param argc The number of command line arguments.
 * @param argv The array of command line argument strings.
 */
void printArgumentAtIndex(int argc, char* argv[]) {
    // 1. Check if there are enough arguments (program name + index)
    if (argc < 2) {
        std::cerr << "Error: Not enough arguments. Usage: <program> <index> [arg1] [arg2] ..." << std::endl;
        return;
    }

    std::string indexString(argv[1]);
    int index;

    // 2. Convert the second argument (at index 1) to an integer
    if (!convertStringToInt(indexString, index)) {
        std::cerr << "Error: The second argument '" << indexString << "' must be a valid integer." << std::endl;
        return;
    }

    // 3. Check if the calculated index is within the bounds of the argv array
    if (index >= 0 && index < argc) {
        // 4. Print the argument at the specified index
        std::cout << argv[index] << std::endl;
    } else {
        std::cerr << "Error: Index " << index << " is out of bounds. Valid range is 0 to " << (argc - 1) << "." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "--- Running with actual command line arguments ---" << std::endl;
    // To test this part, compile and run from command line:
    // ./a.out 3 apple banana orange grape
    // Expected output: grape
    if (argc > 1) {
        printArgumentAtIndex(argc, argv);
    } else {
        std::cout << "No command line arguments provided for live test." << std::endl;
    }

    std::cout << "\n--- Running with 5 test cases ---" << std::endl;

    // Test Case 1: Valid input
    std::cout << "Test 1: ";
    char* test1[] = {(char*)"program", (char*)"3", (char*)"zero", (char*)"one", (char*)"two", (char*)"three"};
    printArgumentAtIndex(6, test1); // Expected: one

    // Test Case 2: Index out of bounds (too high)
    std::cout << "Test 2: ";
    char* test2[] = {(char*)"program", (char*)"5", (char*)"a", (char*)"b", (char*)"c"};
    printArgumentAtIndex(5, test2); // Expected: Error: Index 5 is out of bounds.

    // Test Case 3: Index is not a number
    std::cout << "Test 3: ";
    char* test3[] = {(char*)"program", (char*)"two", (char*)"a", (char*)"b", (char*)"c"};
    printArgumentAtIndex(5, test3); // Expected: Error: The second argument 'two' must be a valid integer.

    // Test Case 4: Not enough arguments
    std::cout << "Test 4: ";
    char* test4[] = {(char*)"program"};
    printArgumentAtIndex(1, test4); // Expected: Error: Not enough arguments.

    // Test Case 5: Index is negative
    std::cout << "Test 5: ";
    char* test5[] = {(char*)"program", (char*)"-1", (char*)"a", (char*)"b"};
    printArgumentAtIndex(4, test5); // Expected: Error: Index -1 is out of bounds.

    return 0;
}