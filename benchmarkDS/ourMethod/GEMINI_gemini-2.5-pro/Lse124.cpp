#include <iostream>
#include <string>
#include <vector>

// Function to process the argument as described
void processArgument(const std::string& input) {
    // The third line prints the argument.
    std::cout << "Original: '" << input << "'" << std::endl;

    // The fourth line trims the trailing whitespace from the argument.
    std::string trimmedInput = input;
    size_t endpos = trimmedInput.find_last_not_of(" \t\n\r\f\v");
    if (std::string::npos != endpos) {
        trimmedInput = trimmedInput.substr(0, endpos + 1);
    } else {
        // String contains only whitespace, so it becomes empty
        trimmedInput.clear();
    }

    // The fifth line prints the argument again.
    std::cout << "Trimmed:  '" << trimmedInput << "'" << std::endl;
    std::cout << "--------------------" << std::endl;
}

// Function to run 5 test cases
void runTests() {
    std::cout << "Running test cases..." << std::endl;
    std::cout << "--------------------" << std::endl;
    processArgument("This is a test.   ");
    processArgument("No trailing space.");
    processArgument("   Leading and trailing spaces.   ");
    processArgument(" \t\n"); // All whitespace
    processArgument("");      // Empty string
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    // argc includes the program name, so we check for 2.
    if (argc != 2) {
        // If not it prints an error message and exits.
        std::cerr << "Error: Please provide exactly one argument." << std::endl;
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program") << " <argument>" << std::endl;

        // Run tests if no arguments are provided to satisfy the prompt.
        if (argc == 1) {
            runTests();
        }

        // The sixth line exits the program.
        return 1; // Exit with an error code
    }

    // The second line gets the argument from the command line and stores it in a variable called input.
    std::string input(argv[1]);

    processArgument(input);

    // The sixth line exits the program.
    return 0; // Exit successfully
}