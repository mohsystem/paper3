
#include <iostream>
#include <string>
#include <cstring>

// Check if str ends with suffix
// Returns true if str ends with suffix, false otherwise
// Security: Uses std::string for safe memory management
// Validates inputs to prevent undefined behavior
bool solution(const std::string& str, const std::string& suffix) {
    // Input validation: check for empty suffix (always matches by definition)
    if (suffix.empty()) {
        return true;
    }
    
    // Input validation: if suffix is longer than str, cannot match
    if (suffix.length() > str.length()) {
        return false;
    }
    
    // Safe comparison using std::string methods
    // Compare the last suffix.length() characters of str with suffix
    // This is bounds-safe as we validated lengths above
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

int main() {
    // Test case 1: Basic suffix match
    std::cout << "Test 1: solution('abc', 'bc') = " 
              << (solution("abc", "bc") ? "true" : "false") 
              << " (expected: true)" << std::endl;
    
    // Test case 2: No match
    std::cout << "Test 2: solution('abc', 'd') = " 
              << (solution("abc", "d") ? "true" : "false") 
              << " (expected: false)" << std::endl;
    
    // Test case 3: Empty suffix (should return true)
    std::cout << "Test 3: solution('abc', '') = " 
              << (solution("abc", "") ? "true" : "false") 
              << " (expected: true)" << std::endl;
    
    // Test case 4: Suffix longer than string
    std::cout << "Test 4: solution('ab', 'abc') = " 
              << (solution("ab", "abc") ? "true" : "false") 
              << " (expected: false)" << std::endl;
    
    // Test case 5: Exact match (whole string is suffix)
    std::cout << "Test 5: solution('hello', 'hello') = " 
              << (solution("hello", "hello") ? "true" : "false") 
              << " (expected: true)" << std::endl;
    
    return 0;
}
