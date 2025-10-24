#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Processes the given string by printing it and its length.
 * 
 * This function is inherently safe from buffer overflows as it uses
 * std::string, which manages its own memory.
 * 
 * @param input The string to be processed.
 * @param test_case_num The number of the current test case.
 */
void processString(const std::string& input, int test_case_num) {
    std::cout << "\n--- Processing Test Case " << test_case_num << " ---" << std::endl;
    std::cout << "Input string: \"" << input << "\"" << std::endl;
    std::cout << "Length: " << input.length() << std::endl;
    std::cout << "---------------------------------" << std::endl;
}

/**
 * @brief Main function to demonstrate safe string input in C++.
 * 
 * It runs 5 test cases where the user provides input, which is read
 * safely using std::getline and std::string.
 * 
 * @return int Exit code. 0 for success.
 */
int main() {
    std::cout << "C++ Safe String Input Demonstration" << std::endl;
    std::cout << "Please provide 5 string inputs for processing." << std::endl;
    std::cout << "==========================================" << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\nEnter test string #" << i << ": ";
        std::string userInput;

        // std::getline reads an entire line from std::cin into a std::string.
        // It automatically handles memory allocation, making it safe from
        // buffer overflow vulnerabilities.
        if (std::getline(std::cin, userInput)) {
            processString(userInput, i);
        } else {
            // This case handles EOF (e.g., Ctrl+D on Unix, Ctrl+Z on Windows)
            // or a stream error.
            if (std::cin.eof()) {
                std::cout << "\nEnd of file reached. Exiting." << std::endl;
            } else {
                std::cerr << "Error reading input. Exiting." << std::endl;
            }
            break; 
        }
    }
    
    std::cout << "\nProgram finished after 5 tests." << std::endl;

    return 0;
}