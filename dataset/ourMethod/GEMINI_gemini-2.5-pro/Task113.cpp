#include <iostream>
#include <string>
#include <vector>
#include <numeric>

/**
 * @brief Concatenates a vector of strings.
 *
 * This function calculates the total required size for the final string,
 * reserves the memory in advance to avoid reallocations, and then appends
 * each string from the input vector.
 *
 * @param strings A constant reference to a vector of std::string.
 * @return A std::string containing the concatenation of all input strings.
 */
std::string concatenateStrings(const std::vector<std::string>& strings) {
    size_t total_length = 0;
    for (const auto& s : strings) {
        total_length += s.length();
    }

    std::string result;
    // Reserve memory to avoid multiple reallocations for better performance.
    result.reserve(total_length);

    for (const auto& s : strings) {
        result.append(s);
    }

    return result;
}

void run_test(const std::string& test_name, const std::vector<std::string>& input) {
    std::cout << test_name << ": " << concatenateStrings(input) << std::endl;
}

int main() {
    // Test Case 1: Basic concatenation
    std::vector<std::string> test1 = {"Hello", ", ", "World", "!"};
    run_test("Test Case 1", test1);

    // Test Case 2: Vector with empty strings
    std::vector<std::string> test2 = {"C++", " ", "is", "", " efficient."};
    run_test("Test Case 2", test2);

    // Test Case 3: Empty vector
    std::vector<std::string> test3 = {};
    run_test("Test Case 3", test3);

    // Test Case 4: Vector with a mix of strings
    std::vector<std::string> test4 = {"alpha", "beta", "gamma"};
    run_test("Test Case 4", test4);

    // Test Case 5: Single string
    std::vector<std::string> test5 = {"SingleString"};
    run_test("Test Case 5", test5);

    return 0;
}