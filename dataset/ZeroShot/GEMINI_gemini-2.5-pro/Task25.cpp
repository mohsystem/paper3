#include <iostream>
#include <vector>
#include <string>

/**
 * Prepends line numbers to a vector of strings.
 * The numbering starts at 1. The format is n: string.
 *
 * @param lines The vector of strings to be numbered.
 * @return A new vector of strings with each line prepended by its number.
 */
std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> numberedLines;
    if (lines.empty()) {
        return numberedLines;
    }
    
    numberedLines.reserve(lines.size()); // Pre-allocate memory for efficiency
    int lineNumber = 1;
    for (const auto& line : lines) {
        numberedLines.push_back(std::to_string(lineNumber) + ": " + line);
        lineNumber++;
    }
    return numberedLines;
}

// Helper function to print a vector of strings
void print_vector(const std::string& label, const std::vector<std::string>& vec) {
    std::cout << label << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

int main() {
    // Test Case 1: Empty list
    std::vector<std::string> test1 = {};
    print_vector("Test Case 1 Input: ", test1);
    std::cout << std::endl;
    print_vector("Output: ", number(test1));
    std::cout << std::endl << std::endl;

    // Test Case 2: Simple list
    std::vector<std::string> test2 = {"a", "b", "c"};
    print_vector("Test Case 2 Input: ", test2);
    std::cout << std::endl;
    print_vector("Output: ", number(test2));
    std::cout << std::endl << std::endl;

    // Test Case 3: List with an empty string
    std::vector<std::string> test3 = {"", "hello", ""};
    print_vector("Test Case 3 Input: ", test3);
    std::cout << std::endl;
    print_vector("Output: ", number(test3));
    std::cout << std::endl << std::endl;

    // Test Case 4: List with longer strings
    std::vector<std::string> test4 = {"First line", "Second line", "Third line"};
    print_vector("Test Case 4 Input: ", test4);
    std::cout << std::endl;
    print_vector("Output: ", number(test4));
    std::cout << std::endl << std::endl;

    // Test Case 5: List with one element
    std::vector<std::string> test5 = {"single"};
    print_vector("Test Case 5 Input: ", test5);
    std::cout << std::endl;
    print_vector("Output: ", number(test5));
    std::cout << std::endl << std::endl;

    return 0;
}