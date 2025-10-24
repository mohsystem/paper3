
#include <iostream>
#include <string>
#include <set>
#include <stdexcept>

// Function to merge two strings and return sorted distinct letters
// Security: Input validation ensures only lowercase letters a-z are processed
std::string longest(const std::string& s1, const std::string& s2) {
    // Validate inputs - only allow lowercase letters a-z
    // Prevents injection of control characters or invalid data
    for (char c : s1) {
        if (c < 'a' || c > 'z') {
            throw std::invalid_argument("s1 contains invalid characters");
        }
    }
    for (char c : s2) {
        if (c < 'a' || c > 'z') {
            throw std::invalid_argument("s2 contains invalid characters");
        }
    }
    
    // Use std::set for automatic sorting and uniqueness
    // Avoids manual memory management and buffer overflow risks
    std::set<char> uniqueChars;
    
    // Insert all characters from both strings
    // Set automatically handles duplicates
    for (char c : s1) {
        uniqueChars.insert(c);
    }
    for (char c : s2) {
        uniqueChars.insert(c);
    }
    
    // Build result string from set
    // String handles memory safely with RAII
    std::string result;
    result.reserve(uniqueChars.size()); // Pre-allocate to avoid reallocations
    
    for (char c : uniqueChars) {
        result += c;
    }
    
    return result;
}

int main() {
    // Test case 1
    try {
        std::string result1 = longest("xyaabbbccccdefww", "xxxxyyyyabklmopq");
        std::cout << "Test 1: " << result1 << std::endl;
        std::cout << "Expected: abcdefklmopqwxy" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2
    try {
        std::string result2 = longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz");
        std::cout << "\\nTest 2: " << result2 << std::endl;
        std::cout << "Expected: abcdefghijklmnopqrstuvwxyz" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: Empty strings
    try {
        std::string result3 = longest("", "");
        std::cout << "\\nTest 3: " << result3 << std::endl;
        std::cout << "Expected: (empty)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: One empty string
    try {
        std::string result4 = longest("abc", "");
        std::cout << "\\nTest 4: " << result4 << std::endl;
        std::cout << "Expected: abc" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: No common characters
    try {
        std::string result5 = longest("abc", "def");
        std::cout << "\\nTest 5: " << result5 << std::endl;
        std::cout << "Expected: abcdef" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
