#include <iostream>
#include <string>
#include <vector>

/**
 * Processes a single command-line argument based on the prompt's logic.
 *
 * @param argc The argument count.
 * @param argv The argument vector.
 */
void processArgument(int argc, char* argv[]) {
    // 1. Check if exactly one argument is provided. (argc should be 2: program name + arg)
    if (argc != 2) {
        std::cerr << "Error: Please provide exactly one argument." << std::endl;
        // 6. Exit the logical flow.
        return;
    }

    // 2. Get the argument.
    std::string input(argv[1]);

    // 3. Print the original argument.
    std::cout << "Original: '" << input << "'" << std::endl;

    // 4. Trim the trailing whitespace from the argument.
    std::string trimmedInput = input;
    size_t endpos = trimmedInput.find_last_not_of(" \t\n\r\f\v");
    if (std::string::npos != endpos) {
        trimmedInput.erase(endpos + 1);
    } else {
        // String is all whitespace
        trimmedInput.clear();
    }

    // 5. Print the argument again.
    std::cout << "Trimmed: '" << trimmedInput << "'" << std::endl;
    
    // 6. The function ends, effectively exiting this logical block.
}

int main() {
    std::cout << "--- Test Case 1: No arguments ---" << std::endl;
    char* argv1[] = { (char*)"program", nullptr };
    processArgument(1, argv1);

    std::cout << "\n--- Test Case 2: Two arguments ---" << std::endl;
    char* argv2[] = { (char*)"program", (char*)"first", (char*)"second", nullptr };
    processArgument(3, argv2);

    std::cout << "\n--- Test Case 3: One argument, no trailing space ---" << std::endl;
    char* argv3[] = { (char*)"program", (char*)"hello", nullptr };
    processArgument(2, argv3);

    std::cout << "\n--- Test Case 4: One argument with trailing spaces ---" << std::endl;
    char* argv4[] = { (char*)"program", (char*)"world  ", nullptr };
    processArgument(2, argv4);

    std::cout << "\n--- Test Case 5: One argument with leading and trailing spaces ---" << std::endl;
    char* argv5[] = { (char*)"program", (char*)"  spaced out   ", nullptr };
    processArgument(2, argv5);

    return 0;
}