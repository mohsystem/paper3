#include <iostream>
#include <string> // Required for std::stoi
#include <vector> // Required for std::vector

const int OFFSET = 1000;

/**
 * Adds a constant offset to the given integer value.
 * @param value The integer input.
 * @return The value with the offset added.
 */
int addOffset(int value) {
    return value + OFFSET;
}

void runTests() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    std::vector<int> test_cases = {10, 0, -50, 2000, -1000};
    for (int test_case : test_cases) {
        std::cout << "Input: " << test_case << ", Output: " << addOffset(test_case) << std::endl;
    }
    std::cout << "--------------------------\n" << std::endl;
}

int main(int argc, char* argv[]) {
    runTests();

    // Handling command line input
    if (argc > 1) {
        try {
            int inputValue = std::stoi(argv[1]);
            int result = addOffset(inputValue);
            std::cout << "Command line input: " << inputValue << std::endl;
            std::cout << "Result with offset: " << result << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: The provided command line argument is not a valid integer." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: The provided command line argument is out of range." << std::endl;
        }
    } else {
        std::cout << "Usage: " << argv[0] << " <integer_value>" << std::endl;
    }

    return 0;
}