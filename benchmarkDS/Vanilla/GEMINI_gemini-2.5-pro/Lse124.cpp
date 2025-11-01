#include <iostream>
#include <string>
#include <vector>

/**
 * Processes a single command-line argument according to the specified logic.
 * @param argc The argument count.
 * @param argv The argument values.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    // 1) Check for exactly one argument. If not, print an error and exit.
    // argc includes the program name, so it should be 2 for one user argument.
    if (argc != 2) {
        std::cerr << "Error: Please provide exactly one argument." << std::endl;
        return 1; // Exit with an error code
    }

    // 2) Get the argument and store it.
    std::string input(argv[1]);

    // 3) Print the original argument.
    std::cout << "Original: '" << input << "'" << std::endl;

    // 4) Trim the trailing whitespace.
    std::string trimmedInput = input;
    size_t endpos = trimmedInput.find_last_not_of(" \t\n\r\f\v");
    if (std::string::npos != endpos) {
        trimmedInput.erase(endpos + 1);
    } else {
        // String is all whitespace
        trimmedInput.clear();
    }
    
    // 5) Print the trimmed argument.
    std::cout << "Trimmed:  '" << trimmedInput << "'" << std::endl;

    // 6) Exit the program.
    return 0; // Exit successfully

    /*
     * Main method with 5 test cases (to be run from the command line).
     *
     * How to compile and run:
     * g++ your_source_file.cpp -o Lse124
     *
     * Test Case 1: Argument with trailing spaces
     * ./Lse124 "hello world   "
     * Expected Output:
     * Original: 'hello world   '
     * Trimmed:  'hello world'
     *
     * Test Case 2: Argument with leading and trailing spaces
     * ./Lse124 "  leading and trailing  "
     * Expected Output:
     * Original: '  leading and trailing  '
     * Trimmed:  '  leading and trailing'
     *
     * Test Case 3: Argument with no trailing whitespace
     * ./Lse124 "no-whitespace"
     * Expected Output:
     * Original: 'no-whitespace'
     * Trimmed:  'no-whitespace'
     *
     * Test Case 4: No arguments (error case)
     * ./Lse124
     * Expected Output:
     * Error: Please provide exactly one argument.
     *
     * Test Case 5: Multiple arguments (error case)
     * ./Lse124 "one" "two"
     * Expected Output:
     * Error: Please provide exactly one argument.
     */
}