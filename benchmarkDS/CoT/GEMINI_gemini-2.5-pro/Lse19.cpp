#include <iostream>
#include <string>
#include <stdexcept>

/**
 * @brief Parses a C-style string to a long long, adds 1000.
 * @param input The C-style string to be converted.
 * @return The parsed number plus 1000.
 * @throws std::invalid_argument if no conversion could be performed.
 * @throws std::out_of_range if the converted value would fall out of the range of the result type.
 */
long long addThousand(const char* input) {
    // std::stoll is a safe way to convert string to long long, throwing exceptions on failure.
    long long number = std::stoll(input);
    return number + 1000;
}

void runTestCases() {
    std::cout << "\n--- Running 5 Internal Test Cases ---" << std::endl;
    const char* testInputs[] = {"250", "-250", "0", "9223372036854774807", "not-a-number"};
    for (int i = 0; i < 5; ++i) {
        std::cout << "\nTest Case " << (i + 1) << ": Input = \"" << testInputs[i] << "\"" << std::endl;
        try {
            long long testResult = addThousand(testInputs[i]);
            std::cout << "  Output: " << testResult << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << "  Output: Error - Invalid number format." << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "  Output: Error - Number is out of range." << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // --- 1. Process Command Line Argument ---
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <number>" << std::endl;
        std::cerr << "Note: Exactly one command-line argument is required." << std::endl;
    } else {
        try {
            long long result = addThousand(argv[1]);
            std::cout << "Result from command line input '" << argv[1] << "': " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: The provided command-line argument '" << argv[1] << "' is not a valid number." << std::endl;
        }
    }

    // --- 2. Run 5 Test Cases ---
    runTestCases();

    return 0;
}