
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>
#include <limits>

// Security: Input validation - check if character is lowercase letter
// Prevents potential integer overflow or undefined behavior from invalid chars
bool isValidLowercase(char c) {
    return c >= 'a' && c <= 'z';
}

// Security: Safe score calculation with overflow protection
// Returns false if overflow would occur, true otherwise
bool calculateWordScore(const std::string& word, uint32_t& score) {
    // Security: Validate input is not empty
    if (word.empty()) {
        score = 0;
        return true;
    }
    
    // Security: Check maximum possible score won't overflow\n    // Max score per char is 26, so max total is 26 * word.length()\n    if (word.length() > UINT32_MAX / 26) {\n        return false; // Potential overflow\n    }\n    \n    score = 0;\n    for (size_t i = 0; i < word.length(); ++i) {\n        // Security: Validate each character is lowercase letter\n        if (!isValidLowercase(word[i])) {\n            return false;\n        }\n        \n        uint32_t charScore = static_cast<uint32_t>(word[i] - 'a' + 1);
        
        // Security: Check for overflow before addition
        if (score > UINT32_MAX - charScore) {
            return false;
        }
        
        score += charScore;
    }
    
    return true;
}

std::string highestScoringWord(const std::string& input) {
    // Security: Validate input is not too large to prevent DoS
    const size_t MAX_INPUT_LENGTH = 1000000;
    if (input.length() > MAX_INPUT_LENGTH) {
        return ""; // Input too large
    }
    
    // Security: Use vector with bounds checking for word storage
    std::vector<std::string> words;
    std::stringstream ss(input);
    std::string word;
    
    // Security: Limit number of words to prevent memory exhaustion
    const size_t MAX_WORDS = 10000;
    
    // Parse words separated by spaces
    while (ss >> word && words.size() < MAX_WORDS) {
        // Security: Validate word length to prevent excessive processing
        if (word.length() > 1000) {
            continue; // Skip overly long words
        }
        words.push_back(word);
    }
    
    // Security: Handle empty input case
    if (words.empty()) {
        return "";
    }
    
    std::string highestWord;
    uint32_t highestScore = 0;
    bool foundValid = false;
    
    // Find highest scoring word
    for (size_t i = 0; i < words.size(); ++i) {
        uint32_t currentScore = 0;
        
        // Security: Safe score calculation with validation
        if (!calculateWordScore(words[i], currentScore)) {
            continue; // Skip invalid words
        }
        
        // First valid word or higher score found
        if (!foundValid || currentScore > highestScore) {
            highestScore = currentScore;
            highestWord = words[i];
            foundValid = true;
        }
    }
    
    return highestWord;
}

int main() {
    // Test case 1: Basic example
    std::cout << "Test 1: \\"" << highestScoringWord("man i need a taxi up to ubud") << "\\"" << std::endl;
    
    // Test case 2: Single word
    std::cout << "Test 2: \\"" << highestScoringWord("hello") << "\\"" << std::endl;
    
    // Test case 3: Same score, return first
    std::cout << "Test 3: \\"" << highestScoringWord("aa b") << "\\"" << std::endl;
    
    // Test case 4: Different scores
    std::cout << "Test 4: \\"" << highestScoringWord("a bb ccc") << "\\"" << std::endl;
    
    // Test case 5: Empty string edge case
    std::cout << "Test 5: \\"" << highestScoringWord("") << "\\"" << std::endl;
    
    return 0;
}
