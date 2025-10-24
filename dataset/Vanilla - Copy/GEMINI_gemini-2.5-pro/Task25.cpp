#include <iostream>
#include <vector>
#include <string>

/**
 * Takes a vector of strings and returns each line prepended by the correct number.
 * The numbering starts at 1. The format is n: string.
 * @param lines The vector of strings.
 * @return A new vector of strings with line numbers.
 */
std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> result;
    for (size_t i = 0; i < lines.size(); ++i) {
        result.push_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    return result;
}

void print_vector(const std::string& label, const std::vector<std::string>& vec) {
    std::cout << label << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

int main() {
    // Test Case 1: Basic case
    std::cout << "Test Case 1" << std::endl;
    std::vector<std::string> input1 = {"a", "b", "c"};
    print_vector("Input:  ", input1);
    print_vector("Output: ", number(input1));
    std::cout << std::endl;

    // Test Case 2: Empty list
    std::cout << "Test Case 2" << std::endl;
    std::vector<std::string> input2 = {};
    print_vector("Input:  ", input2);
    print_vector("Output: ", number(input2));
    std::cout << std::endl;

    // Test Case 3: List with empty strings
    std::cout << "Test Case 3" << std::endl;
    std::vector<std::string> input3 = {"", "x", ""};
    print_vector("Input:  ", input3);
    print_vector("Output: ", number(input3));
    std::cout << std::endl;

    // Test Case 4: List with multi-word strings
    std::cout << "Test Case 4" << std::endl;
    std::vector<std::string> input4 = {"hello world", "goodbye world"};
    print_vector("Input:  ", input4);
    print_vector("Output: ", number(input4));
    std::cout << std::endl;

    // Test Case 5: Single element list
    std::cout << "Test Case 5" << std::endl;
    std::vector<std::string> input5 = {"Just one line."};
    print_vector("Input:  ", input5);
    print_vector("Output: ", number(input5));
    std::cout << std::endl;

    return 0;
}