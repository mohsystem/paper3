#include <iostream>
#include <string>
#include <vector>
#include <climits> // For LLONG_MAX, LLONG_MIN

/**
 * @brief Adds 1000 to the given value.
 * @param value The input number (long long).
 * @return The input number plus 1000.
 */
long long addThousand(long long value) {
    return value + 1000;
}

/**
 * @brief Runs a set of predefined test cases to verify the addThousand logic.
 */
void runTestCases() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    long long testInputs[] = {50, 0, -50, LLONG_MAX - 1000, LLONG_MIN};
    long long expectedOutputs[] = {1050, 1000, 950, LLONG_MAX, LLONG_MIN + 1000};

    for (int i = 0; i < 5; ++i) {
        long long result = addThousand(testInputs[i]);
        std::cout << "Test " << i + 1 << ": Input=" << testInputs[i]
                  << ", Output=" << result << ", Expected=" << expectedOutputs[i]
                  << ", Pass=" << (result == expectedOutputs[i] ? "true" : "false") << std::endl;
    }
    std::cout << "--- Test Cases Finished ---" << std::endl << std::endl;
}

/**
 * @brief Main function. Runs test cases and processes a command-line argument.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on completion.
 */
int main(int argc, char* argv[]) {
    // Run test cases to demonstrate functionality
    runTestCases();

    std::cout << "--- Processing Command-Line Argument ---" << std::endl;
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number>" << std::endl;
        std::cerr << "Please provide exactly one numeric argument." << std::endl;
        return 0; // As per prompt, return 0 even on usage error
    }

    try {
        std::string arg_str = argv[1];
        long long inputValue = std::stoll(arg_str);
        long long result = addThousand(inputValue);
        std::cout << "Result from command-line input: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: The provided argument '" << argv[1] << "' is not a valid number." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: The provided argument '" << argv[1] << "' is out of the valid range." << std::endl;
    }

    return 0;
}