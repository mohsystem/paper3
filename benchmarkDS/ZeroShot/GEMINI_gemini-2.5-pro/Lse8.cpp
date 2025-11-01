#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * Securely retrieves an argument from the array based on an index provided in argv[1].
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The string at the specified index or an error message.
 */
std::string getArgumentAtIndex(int argc, char* argv[]) {
    // Security Check 1: Ensure there are enough arguments to get an index.
    if (argc < 2) {
        return "Error: Not enough arguments. An index must be provided as the second argument.";
    }

    int index;
    try {
        // Security Check 2: Use std::stoi for safe string-to-int conversion.
        index = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        return "Error: Index provided in the second argument must be a valid integer.";
    } catch (const std::out_of_range& e) {
        return "Error: Index value is out of range for an integer.";
    }

    // Security Check 3: Ensure the index is within the valid bounds of the array.
    if (index < 0) {
        return "Error: Index cannot be negative.";
    }
    if (index >= argc) {
        return "Error: Index " + std::to_string(index) + " is out of bounds for " + std::to_string(argc) + " arguments.";
    }

    // If all checks pass, return the value at the specified index.
    return std::string(argv[index]);
}

int main() {
    std::cout << "--- C++: Testing argument retrieval ---" << std::endl;

    // Test Case 1: Valid input
    char* test1_args[] = { (char*)"./Lse8", (char*)"3", (char*)"apple", (char*)"banana" };
    std::cout << "Test Case 1 Input: {\"./Lse8\", \"3\", \"apple\", \"banana\"}" << std::endl;
    std::cout << "Result: " << getArgumentAtIndex(4, test1_args) << std::endl << std::endl;

    // Test Case 2: Not enough arguments
    char* test2_args[] = { (char*)"./Lse8" };
    std::cout << "Test Case 2 Input: {\"./Lse8\"}" << std::endl;
    std::cout << "Result: " << getArgumentAtIndex(1, test2_args) << std::endl << std::endl;

    // Test Case 3: Invalid index format (not an integer)
    char* test3_args[] = { (char*)"./Lse8", (char*)"abc", (char*)"arg2", (char*)"arg3" };
    std::cout << "Test Case 3 Input: {\"./Lse8\", \"abc\", \"arg2\", \"arg3\"}" << std::endl;
    std::cout << "Result: " << getArgumentAtIndex(4, test3_args) << std::endl << std::endl;

    // Test Case 4: Index out of bounds (too high)
    char* test4_args[] = { (char*)"./Lse8", (char*)"10", (char*)"arg2" };
    std::cout << "Test Case 4 Input: {\"./Lse8\", \"10\", \"arg2\"}" << std::endl;
    std::cout << "Result: " << getArgumentAtIndex(3, test4_args) << std::endl << std::endl;

    // Test Case 5: Index out of bounds (negative)
    char* test5_args[] = { (char*)"./Lse8", (char*)"-1", (char*)"arg2", (char*)"arg3" };
    std::cout << "Test Case 5 Input: {\"./Lse8\", \"-1\", \"arg2\", \"arg3\"}" << std::endl;
    std::cout << "Result: " << getArgumentAtIndex(4, test5_args) << std::endl;
    
    return 0;
}