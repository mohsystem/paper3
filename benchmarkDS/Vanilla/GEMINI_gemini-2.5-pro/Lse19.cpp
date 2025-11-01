#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Adds 1000 to the given integer.
 * @param number The input integer.
 * @return The input number plus 1000.
 */
int addThousand(int number) {
    return number + 1000;
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, process it.
    if (argc > 1) {
        try {
            int inputNumber = std::stoi(argv[1]);
            int result = addThousand(inputNumber);
            std::cout << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Input must be a valid integer." << std::endl;
            return 1;
        }
    } else {
        // Otherwise, run the 5 built-in test cases.
        std::cout << "No command-line argument provided. Running test cases:" << std::endl;
        std::vector<int> testCases = {10, -50, 0, 12345, -1000};
        for (int testInput : testCases) {
            int result = addThousand(testInput);
            std::cout << "Input: " << testInput << ", Output: " << result << std::endl;
        }
    }
    return 0;
}