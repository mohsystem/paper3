#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Converts the second command-line argument to an integer (INDEX) and prints
 * the argument at that INDEX.
 *
 * @param argc The count of command-line arguments.
 * @param argv The array of command-line arguments.
 */
void processAndPrintArgument(int argc, char* argv[]) {
    // 1. Check if the second argument (for the index) exists.
    if (argc < 2) {
        std::cerr << "Error: Not enough arguments. Please provide an index." << std::endl;
        return;
    }

    std::string indexStr(argv[1]);
    int index;

    // 2. Securely convert the string to an integer.
    try {
        size_t pos; // To check if the whole string was consumed
        index = std::stoi(indexStr, &pos);
        // Check for trailing characters
        if (pos != indexStr.length()) {
            throw std::invalid_argument("Trailing characters");
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: The provided index '" << indexStr << "' is not a valid integer." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: The provided index '" << indexStr << "' is out of the integer range." << std::endl;
        return;
    }

    // 3. Check if the parsed index is within the valid bounds of the arguments array.
    if (index < 0 || index >= argc) {
        std::cerr << "Error: Index " << index << " is out of bounds. Must be between 0 and " << (argc - 1) << "." << std::endl;
        return;
    }

    // 4. Print the argument at the specified index.
    std::cout << "Argument at index " << index << ": " << argv[index] << std::endl;
}

int main() {
    std::cout << "--- Test Cases ---" << std::endl;

    // Test Case 1: Valid input
    std::cout << "\nTest Case 1: ./Lse7 2 Hello World" << std::endl;
    char* test1[] = {(char*)"./Lse7", (char*)"2", (char*)"Hello", (char*)"World"};
    processAndPrintArgument(4, test1);

    // Test Case 2: Index out of bounds (too high)
    std::cout << "\nTest Case 2: ./Lse7 10 Hello" << std::endl;
    char* test2[] = {(char*)"./Lse7", (char*)"10", (char*)"Hello"};
    processAndPrintArgument(3, test2);

    // Test Case 3: Invalid number format for index
    std::cout << "\nTest Case 3: ./Lse7 abc Hello" << std::endl;
    char* test3[] = {(char*)"./Lse7", (char*)"abc", (char*)"Hello"};
    processAndPrintArgument(3, test3);

    // Test Case 4: Not enough arguments to get the index
    std::cout << "\nTest Case 4: ./Lse7" << std::endl;
    char* test4[] = {(char*)"./Lse7"};
    processAndPrintArgument(1, test4);

    // Test Case 5: Integer overflow for the index
    std::cout << "\nTest Case 5: ./Lse7 2147483648" << std::endl;
    char* test5[] = {(char*)"./Lse7", (char*)"2147483648"};
    processAndPrintArgument(2, test5);

    return 0;
}