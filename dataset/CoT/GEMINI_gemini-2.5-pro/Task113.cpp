#include <iostream>
#include <vector>
#include <string>

/**
 * @brief Concatenates multiple strings from a vector.
 *
 * This function is secure against buffer overflows by using std::string,
 * which handles memory management automatically. It also reserves memory
 * in advance as a performance optimization to prevent multiple reallocations.
 *
 * @param strings A constant reference to a vector of std::string.
 * @return A single std::string containing the concatenation of all input strings.
 */
std::string concatenateStrings(const std::vector<std::string>& strings) {
    std::string result;
    
    // Optimization: Calculate total size and reserve memory to avoid reallocations.
    size_t total_size = 0;
    for (const std::string& s : strings) {
        total_size += s.length();
    }
    result.reserve(total_size);
    
    // Append each string to the result.
    for (const std::string& s : strings) {
        result.append(s);
    }
    
    return result;
}

void runTestCase(const std::string& testName, const std::vector<std::string>& testData) {
    std::cout << testName << ": [";
    for (size_t i = 0; i < testData.size(); ++i) {
        std::cout << "\"" << testData[i] << "\"";
        if (i < testData.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
    std::cout << "Result: " << concatenateStrings(testData) << std::endl << std::endl;
}

int main() {
    // Test Case 1: Basic concatenation
    std::vector<std::string> test1 = {"Hello", " ", "World", "!"};
    runTestCase("Test Case 1", test1);
    
    // Test Case 2: Another basic case
    std::vector<std::string> test2 = {"C++", " ", "is", " ", "powerful."};
    runTestCase("Test Case 2", test2);
    
    // Test Case 3: Empty vector
    std::vector<std::string> test3 = {};
    runTestCase("Test Case 3", test3);
    
    // Test Case 4: Vector with a single element
    std::vector<std::string> test4 = {"SingleString"};
    runTestCase("Test Case 4", test4);
    
    // Test Case 5: Vector with empty strings
    std::vector<std::string> test5 = {"part1", "", "part2", "", "part3"};
    runTestCase("Test Case 5", test5);

    return 0;
}