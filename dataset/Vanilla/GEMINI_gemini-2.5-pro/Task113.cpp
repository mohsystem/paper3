#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/**
 * @brief Concatenates a vector of strings into a single string.
 *
 * @param strings The vector of strings to concatenate.
 * @return The single concatenated string.
 */
std::string concatenateStrings(const std::vector<std::string>& strings) {
    std::stringstream ss;
    for (const auto& s : strings) {
        ss << s;
    }
    return ss.str();
}

int main() {
    // Test Case 1
    std::vector<std::string> test1 = {"Hello", " ", "World", "!"};
    std::cout << "Test Case 1 Input: {\"Hello\", \" \", \"World\", \"!\"}" << std::endl;
    std::cout << "Output: " << concatenateStrings(test1) << std::endl;
    std::cout << std::endl;

    // Test Case 2
    std::vector<std::string> test2 = {"Java", ", ", "Python", ", ", "C++", ", ", "C"};
    std::cout << "Test Case 2 Input: {\"Java\", \", \", \"Python\", \", \", \"C++\", \", \", \"C\"}" << std::endl;
    std::cout << "Output: " << concatenateStrings(test2) << std::endl;
    std::cout << std::endl;

    // Test Case 3
    std::vector<std::string> test3 = {"One"};
    std::cout << "Test Case 3 Input: {\"One\"}" << std::endl;
    std::cout << "Output: " << concatenateStrings(test3) << std::endl;
    std::cout << std::endl;

    // Test Case 4: With empty strings
    std::vector<std::string> test4 = {"", "leading", " and ", "trailing", ""};
    std::cout << "Test Case 4 Input: {\"\", \"leading\", \" and \", \"trailing\", \"\"}" << std::endl;
    std::cout << "Output: " << concatenateStrings(test4) << std::endl;
    std::cout << std::endl;

    // Test Case 5: Empty input vector
    std::vector<std::string> test5 = {};
    std::cout << "Test Case 5 Input: {}" << std::endl;
    std::cout << "Output: " << concatenateStrings(test5) << std::endl;
    std::cout << std::endl;

    return 0;
}