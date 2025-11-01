#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Adds 1000 to the given number.
 * 
 * @param number The integer to which 1000 will be added.
 * @return The sum of the number and 1000.
 */
int addThousand(int number) {
    return number + 1000;
}

/**
 * @brief Main function. If a command-line argument is provided, it adds 1000 to it and prints the result.
 * Otherwise, it runs 5 hardcoded test cases.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line arguments.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    if (argc > 1) {
        try {
            int inputVal = std::stoi(argv[1]);
            int result = addThousand(inputVal);
            std::cout << result << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "Error: The provided argument is not a valid integer." << std::endl;
            return 1;
        } catch (const std::out_of_range& e) {
            std::cerr << "Error: Argument out of range for an integer." << std::endl;
            return 1;
        }
    } else {
        std::cout << "No command line argument provided. Running test cases:" << std::endl;
        // 5 test cases
        std::vector<int> testValues = {5, -10, 0, 999, -2000};
        for (int val : testValues) {
            int result = addThousand(val);
            std::cout << "Input: " << val << ", Output: " << result << std::endl;
        }
    }
    return 0;
}