#include <iostream>
#include <string>
#include <vector>

/**
 * Securely concatenates multiple strings using std::string.
 * std::string handles memory management automatically, preventing buffer overflows.
 * A performance optimization is included to reserve memory once.
 *
 * @param strings A constant reference to a vector of strings.
 * @return The concatenated std::string.
 */
std::string concatenateStrings(const std::vector<std::string>& strings) {
    std::string result;
    
    // Optimization: Calculate total size and reserve memory to avoid reallocations.
    size_t total_size = 0;
    for (const auto& s : strings) {
        total_size += s.length();
    }
    result.reserve(total_size);

    // Append all strings.
    for (const auto& s : strings) {
        result.append(s);
    }
    
    return result;
}

void run_test(const std::string& test_name, const std::string& expected, const std::vector<std::string>& input) {
    std::cout << test_name << concatenateStrings(input) << std::endl;
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    // Test Case 1: Basic concatenation
    std::vector<std::string> input1 = {"Hello", ", ", "World", "!"};
    run_test("Test Case 1: ", "Hello, World!", input1);

    // Test Case 2: Concatenating with empty strings
    std::vector<std::string> input2 = {"One", "", "Two", "", "Three"};
    run_test("Test Case 2: ", "OneTwoThree", input2);

    // Test Case 3: Single string
    std::vector<std::string> input3 = {"OnlyOneString"};
    run_test("Test Case 3: ", "OnlyOneString", input3);

    // Test Case 4: No strings (empty vector)
    std::vector<std::string> input4 = {};
    run_test("Test Case 4: ", "", input4);

    // Test Case 5: A mix of longer strings
    std::vector<std::string> input5 = {"C++ ", "with ", "std::string ", "is ", "safe."};
    run_test("Test Case 5: ", "C++ with std::string is safe.", input5);

    return 0;
}