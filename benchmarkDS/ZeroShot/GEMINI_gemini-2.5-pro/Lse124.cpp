#include <iostream>
#include <string>
#include <vector>

/**
 * Processes a single string argument according to the prompt's logic.
 * @param input The string argument to process.
 */
void handle_argument_processing(const std::string& input) {
    // The third line prints the argument.
    std::cout << "Original: [" << input << "]" << std::endl;

    std::string trimmed_input = input;
    // The fourth line trims the trailing whitespace from the argument.
    size_t endpos = trimmed_input.find_last_not_of(" \t\n\r\f\v");
    if (std::string::npos != endpos) {
        trimmed_input.erase(endpos + 1);
    } else {
        // String contains only whitespace or is empty
        trimmed_input.clear();
    }

    // The fifth line prints the argument again.
    std::cout << "Trimmed:  [" << trimmed_input << "]" << std::endl;
}

/**
 * Runs a set of 5 predefined test cases.
 */
void run_test_cases() {
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Test Case 1: 'hello world   '" << std::endl;
    handle_argument_processing("hello world   ");
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Test Case 2: '   leading and trailing   '" << std::endl;
    handle_argument_processing("   leading and trailing   ");
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Test Case 3: 'no_whitespace'" << std::endl;
    handle_argument_processing("no_whitespace");
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Test Case 4: '   ' (only whitespace)" << std::endl;
    handle_argument_processing("   ");
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Test Case 5: '' (empty string)" << std::endl;
    handle_argument_processing("");
    std::cout << "----------------------------------------" << std::endl;
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    // argc includes the program name, so count should be 2.
    if (argc != 2) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program_name") << " <argument>" << std::endl;
        std::cerr << "Error: Exactly one argument is required." << std::endl;

        // If not it prints an error message and exits.
        // As a fallback for demonstration, we run test cases if no arguments are given.
        if (argc == 1) {
            std::cout << "\nRunning built-in test cases as no arguments were provided." << std::endl;
            run_test_cases();
            // The sixth line exits the program.
            return 0; // Clean exit after tests
        } else {
            // The sixth line exits the program (for the error case).
            return 1;
        }
    } else {
        // The second line gets the argument from the command line and stores it.
        std::string input(argv[1]);
        handle_argument_processing(input);
        // The sixth line exits the program.
        return 0;
    }
}