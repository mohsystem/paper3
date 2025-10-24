#include <iostream>
#include <vector>
#include <string>

/**
 * Prepends line numbers to a vector of strings.
 *
 * @param lines The vector of strings to number.
 * @return A new vector of strings with each line prepended by "n: ".
 */
std::vector<std::string> number(const std::vector<std::string>& lines) {
    std::vector<std::string> numberedLines;
    numberedLines.reserve(lines.size()); // Pre-allocate memory for efficiency
    for (size_t i = 0; i < lines.size(); ++i) {
        // The numbering starts at 1, so we use i + 1.
        numberedLines.push_back(std::to_string(i + 1) + ": " + lines[i]);
    }
    return numberedLines;
}

// Helper function to print a vector of strings
void print_vector(const std::string& name, const std::vector<std::string>& vec) {
    std::cout << name << " (Input: [";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << "\"" << vec[i] << "\"";
        if (i < vec.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "])" << std::endl;

    std::vector<std::string> result = number(vec);

    std::cout << "Output: [";
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "\"" << result[i] << "\"";
        if (i < result.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl << std::endl;
}

int main() {
    // Test Case 1: Empty vector
    std::vector<std::string> test1 = {};
    print_vector("Test Case 1", test1);

    // Test Case 2: Standard vector
    std::vector<std::string> test2 = {"a", "b", "c"};
    print_vector("Test Case 2", test2);

    // Test Case 3: Vector with empty strings
    std::vector<std::string> test3 = {"", "", "d"};
    print_vector("Test Case 3", test3);

    // Test Case 4: Vector with two strings
    std::vector<std::string> test4 = {"Hello", "World"};
    print_vector("Test Case 4", test4);

    // Test Case 5: Vector with one string
    std::vector<std::string> test5 = {"single line"};
    print_vector("Test Case 5", test5);

    return 0;
}