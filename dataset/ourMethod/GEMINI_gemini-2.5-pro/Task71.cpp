#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Attempts to convert a string to an integer.
 * 
 * @param s The string to convert.
 * @return The integer value of the string.
 * @throw std::invalid_argument if no conversion could be performed.
 * @throw std::out_of_range if the converted value would fall out of the range of an int.
 */
int convertStringToInt(const std::string& s) {
    return std::stoi(s);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << "Processing command line argument:" << std::endl;
        std::string input_str(argv[1]);
        try {
            int result = convertStringToInt(input_str);
            std::cout << "Successfully converted \"" << input_str << "\" to " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: Could not convert \"" << input_str << "\". " << e.what() << std::endl;
        }
    } else {
        std::cout << "No command line arguments provided. Running test cases:" << std::endl;
        std::vector<std::string> test_cases = {
            "123",          // Valid positive integer
            "-456",         // Valid negative integer
            "0",            // Valid zero
            "abc",          // Invalid format
            "2147483648"    // Out of range for a 32-bit int
        };
        for (const auto& test : test_cases) {
            try {
                int result = convertStringToInt(test);
                std::cout << "Input: \"" << test << "\", Output: " << result << std::endl;
            } catch (const std::exception& e) {
                std::cout << "Input: \"" << test << "\", Error: " << e.what() << std::endl;
            }
        }
    }
    return 0;
}