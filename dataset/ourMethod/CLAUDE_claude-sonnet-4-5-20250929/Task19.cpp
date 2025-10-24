
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <cctype>

// Function to reverse words with 5 or more letters
// Security: Input validation ensures only letters and spaces are processed
// No dynamic memory issues as std::string handles memory safely with RAII
std::string spinWords(const std::string& sentence) {
    // Input validation: check for null/empty input
    if (sentence.empty()) {
        return "";
    }
    
    // Validate input contains only letters and spaces (as per specification)
    // Security measure: prevents injection or unexpected characters
    for (size_t i = 0; i < sentence.length(); ++i) {
        char c = sentence[i];
        if (!std::isalpha(static_cast<unsigned char>(c)) && c != ' ') {
            // Invalid character found, return empty or throw
            return "";
        }
    }
    
    std::string result;
    result.reserve(sentence.length()); // Pre-allocate to avoid reallocation
    
    std::string currentWord;
    currentWord.reserve(64); // Reserve reasonable space for words
    
    // Process each character with bounds checking
    for (size_t i = 0; i < sentence.length(); ++i) {
        char c = sentence[i];
        
        if (c == ' ') {
            // End of word - process it
            if (currentWord.length() >= 5) {
                // Reverse words with 5+ letters using safe STL algorithm
                std::reverse(currentWord.begin(), currentWord.end());
            }
            result += currentWord;
            result += ' ';
            currentWord.clear();
        } else {
            // Build current word, append is bounds-safe
            currentWord += c;
        }
    }
    
    // Process last word (no trailing space)
    if (currentWord.length() >= 5) {
        std::reverse(currentWord.begin(), currentWord.end());
    }
    result += currentWord;
    
    return result;
}

int main() {
    // Test cases with expected outputs
    std::cout << "Test 1: \\"" << spinWords("Hey fellow warriors") << "\\"" << std::endl;
    // Expected: "Hey wollef sroirraw"
    
    std::cout << "Test 2: \\"" << spinWords("This is a test") << "\\"" << std::endl;
    // Expected: "This is a test"
    
    std::cout << "Test 3: \\"" << spinWords("This is another test") << "\\"" << std::endl;
    // Expected: "This is rehtona test"
    
    std::cout << "Test 4: \\"" << spinWords("Welcome") << "\\"" << std::endl;
    // Expected: "emocleW"
    
    std::cout << "Test 5: \\"" << spinWords("a") << "\\"" << std::endl;
    // Expected: "a"
    
    return 0;
}
