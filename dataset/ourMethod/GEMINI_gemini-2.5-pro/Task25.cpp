#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Prepends line numbers to a vector of strings.
 *
 * The numbering starts at 1. The format is "n: string".
 *
 * @param lines A constant reference to a vector of strings.
 * @return A new vector of strings with each line prepended by its number.
 */
std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> numbered_lines;
    // Reserve capacity to avoid reallocations for performance.
    numbered_lines.reserve(lines.size());
    size_t line_number = 1;
    for (const auto& line : lines) {
        // Use std::to_string for safe number-to-string conversion
        // and std::string concatenation.
        numbered_lines.push_back(std::to_string(line_number) + ": " + line);
        line_number++;
    }
    return numbered_lines;
}

/**
 * @brief Helper function to print a vector of strings for testing.
 *
 * @param test_name A descriptive name for the test case.
 * @param vec The vector of strings to print.
 */
void print_vector(const std::string& test_name, const std::vector<std::string>& vec) {
    std::cout << test_name << ":" << std::endl;
    std::cout << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl << std::endl;
}

int main() {
    // Test Case 1: Empty list
    std::vector<std::string> test1_input = {};
    std::vector<std::string> test1_output = number(test1_input);
    print_vector("Test Case 1 (Empty)", test1_output);

    // Test Case 2: Simple list
    std::vector<std::string> test2_input = {"a", "b", "c"};
    std::vector<std::string> test2_output = number(test2_input);
    print_vector("Test Case 2 (Simple)", test2_output);

    // Test Case 3: List with empty strings
    std::vector<std::string> test3_input = {"", "x", ""};
    std::vector<std::string> test3_output = number(test3_input);
    print_vector("Test Case 3 (Empty Strings)", test3_output);

    // Test Case 4: List with more complex strings
    std::vector<std::string> test4_input = {"first line", "second line has spaces", "third;"};
    std::vector<std::string> test4_output = number(test4_input);
    print_vector("Test Case 4 (Complex Strings)", test4_output);

    // Test Case 5: Longer list to test double-digit numbering
    std::vector<std::string> test5_input = {"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k"};
    std::vector<std::string> test5_output = number(test5_input);
    print_vector("Test Case 5 (Double Digits)", test5_output);

    return 0;
}