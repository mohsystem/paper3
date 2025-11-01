#include <iostream>
#include <vector>
#include <string>

/**
 * Finds and prints an element from a vector based on command-line arguments.
 * It handles parsing and validation of the index.
 *
 * @param data The vector of strings to access.
 * @param argc The argument count.
 * @param argv The argument vector, where argv[1] is the index.
 */
void printElementAtIndex(const std::vector<std::string>& data, int argc, char* argv[]) {
    // 1. Security Check: Ensure an argument is provided.
    // Note: argv[0] is the program name, so argc should be at least 2.
    if (argc < 2) {
        std::cerr << "Error: No index provided." << std::endl;
        std::cerr << "Usage: please provide an integer index as an argument." << std::endl;
        return;
    }

    std::string arg = argv[1];
    int index;

    // 2. Security Check: Ensure the argument is a valid integer.
    try {
        size_t pos;
        index = std::stoi(arg, &pos);
        // Check if the entire string was consumed, preventing inputs like "3abc"
        if (pos != arg.length()) {
            throw std::invalid_argument("Trailing characters");
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Argument '" << arg << "' is not a valid integer." << std::endl;
        return;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Argument '" << arg << "' is out of range for an integer." << std::endl;
        return;
    }

    // 3. Security Check: Ensure the index is within the vector bounds.
    if (index < 0 || static_cast<size_t>(index) >= data.size()) {
        std::cerr << "Error: Index " << index << " is out of bounds. Must be between 0 and " << (data.size() - 1) << "." << std::endl;
        return;
    }

    // If all checks pass, print the element.
    std::cout << "Element at index " << index << ": " << data[index] << std::endl;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> data = {"Apple", "Banana", "Cherry", "Date", "Elderberry"};
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Valid index
    std::cout << "Test Case 1 (Input: \"2\"): ";
    char* args1[] = {(char*)"./Lse5", (char*)"2"};
    printElementAtIndex(data, 2, args1);

    // Test Case 2: Out-of-bounds index (too high)
    std::cout << "Test Case 2 (Input: \"10\"): ";
    char* args2[] = {(char*)"./Lse5", (char*)"10"};
    printElementAtIndex(data, 2, args2);

    // Test Case 3: Out-of-bounds index (negative)
    std::cout << "Test Case 3 (Input: \"-1\"): ";
    char* args3[] = {(char*)"./Lse5", (char*)"-1"};
    printElementAtIndex(data, 2, args3);

    // Test Case 4: Non-numeric input
    std::cout << "Test Case 4 (Input: \"abc\"): ";
    char* args4[] = {(char*)"./Lse5", (char*)"abc"};
    printElementAtIndex(data, 2, args4);

    // Test Case 5: No input
    std::cout << "Test Case 5 (Input: empty): ";
    char* args5[] = {(char*)"./Lse5"};
    printElementAtIndex(data, 1, args5);
    
    std::cout << "\n--- Testing with actual command-line arguments ---" << std::endl;
    // This part will use the actual arguments passed when running the program
    if (argc > 1) {
        std::cout << "Actual command-line input: " << argv[1] << std::endl;
        printElementAtIndex(data, argc, argv);
    } else {
        std::cout << "No actual command-line arguments provided. Run like './Lse5 3'" << std::endl;
    }

    return 0;
}