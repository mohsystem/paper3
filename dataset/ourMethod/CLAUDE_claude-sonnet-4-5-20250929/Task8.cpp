
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// Function to find the missing letter in a consecutive sequence
// Security: Input validation ensures array size >= 2 and all elements are valid letters
char findMissingLetter(const std::vector<char>& letters) {
    // Input validation: check minimum size (prevents buffer underflow)
    if (letters.size() < 2) {
        throw std::invalid_argument("Array must contain at least 2 elements");
    }
    
    // Input validation: check maximum size to prevent excessive processing
    if (letters.size() > 26) {
        throw std::invalid_argument("Array size exceeds maximum alphabet length");
    }
    
    // Input validation: verify first character is a letter
    char first = letters[0];
    if (!((first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z'))) {
        throw std::invalid_argument("Invalid character in input");
    }
    
    // Iterate through the array to find the gap
    // Using bounds-checked iteration to prevent buffer overflow
    for (size_t i = 0; i < letters.size() - 1; ++i) {
        // Input validation: ensure current character is valid letter
        char current = letters[i];
        if (!((current >= 'a' && current <= 'z') || (current >= 'A' && current <= 'Z'))) {
            throw std::invalid_argument("Invalid character in input");
        }
        
        // Bounds check: ensure we don't access beyond array size\n        if (i + 1 >= letters.size()) {\n            break;\n        }\n        \n        char next = letters[i + 1];\n        \n        // Input validation: ensure next character is valid letter\n        if (!((next >= 'a' && next <= 'z') || (next >= 'A' && next <= 'Z'))) {\n            throw std::invalid_argument("Invalid character in input");\n        }\n        \n        // Input validation: ensure same case (prevent mixed case attacks)\n        bool currentIsUpper = (current >= 'A' && current <= 'Z');\n        bool nextIsUpper = (next >= 'A' && next <= 'Z');\n        if (currentIsUpper != nextIsUpper) {\n            throw std::invalid_argument("Mixed case letters not allowed");\n        }\n        \n        // Check if there's a gap (missing letter)
        // Safe integer arithmetic: chars are small values, no overflow risk
        int expectedNext = static_cast<int>(current) + 1;
        int actualNext = static_cast<int>(next);
        
        if (actualNext - expectedNext == 1) {
            // Found the missing letter
            return static_cast<char>(expectedNext);
        } else if (actualNext - expectedNext > 1) {
            // Gap too large (multiple missing letters - invalid input)
            throw std::invalid_argument("Multiple letters missing or non-consecutive sequence");
        }
        // If actualNext == expectedNext + 1, continue to next iteration
    }
    
    // If we reach here, no missing letter found (invalid input)
    throw std::invalid_argument("No missing letter found in sequence");
}

int main() {
    // Test case 1: lowercase missing 'e'
    try {
        std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
        std::cout << "Test 1: " << findMissingLetter(test1) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: uppercase missing 'P'
    try {
        std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
        std::cout << "Test 2: " << findMissingLetter(test2) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: lowercase missing 'n'
    try {
        std::vector<char> test3 = {'m', 'o', 'p'};
        std::cout << "Test 3: " << findMissingLetter(test3) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: uppercase missing 'B'
    try {
        std::vector<char> test4 = {'A', 'C', 'D', 'E'};
        std::cout << "Test 4: " << findMissingLetter(test4) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: lowercase missing 'x'
    try {
        std::vector<char> test5 = {'w', 'y', 'z'};
        std::cout << "Test 5: " << findMissingLetter(test5) << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
