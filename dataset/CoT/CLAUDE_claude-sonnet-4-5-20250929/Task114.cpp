
#include <iostream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <cstring>

class Task114 {
public:
    // Copy string safely
    static std::string copyString(const std::string& source) {
        return std::string(source);
    }
    
    // Concatenate strings safely with validation
    static std::string concatenateStrings(const std::string& str1, const std::string& str2) {
        std::string result;
        result.reserve(str1.length() + str2.length());
        result.append(str1);
        result.append(str2);
        return result;
    }
    
    // Reverse string safely
    static std::string reverseString(const std::string& input) {
        if (input.empty()) {
            return input;
        }
        
        std::string reversed = input;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
    
    // Convert to uppercase safely
    static std::string toUpperCaseSafe(const std::string& input) {
        std::string result = input;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        return result;
    }
    
    // Convert to lowercase safely
    static std::string toLowerCaseSafe(const std::string& input) {
        std::string result = input;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }
    
    // Get substring with bounds checking
    static std::string getSubstringSafe(const std::string& input, size_t start, size_t end) {
        if (start > input.length() || end > input.length() || start > end) {
            throw std::out_of_range("Invalid substring indices");
        }
        return input.substr(start, end - start);
    }
    
    // Replace substring safely
    static std::string replaceSubstring(const std::string& input, 
                                       const std::string& target, 
                                       const std::string& replacement) {
        if (target.empty()) {
            return input;
        }
        
        std::string result = input;
        size_t pos = 0;
        while ((pos = result.find(target, pos)) != std::string::npos) {
            result.replace(pos, target.length(), replacement);
            pos += replacement.length();
        }
        return result;
    }
};

int main() {
    std::cout << "=== String Operations Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Copy String
    std::cout << "Test Case 1 - Copy String:" << std::endl;
    std::string original = "Hello, World!";
    std::string copied = Task114::copyString(original);
    std::cout << "Original: " << original << std::endl;
    std::cout << "Copied: " << copied << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Concatenate Strings
    std::cout << "Test Case 2 - Concatenate Strings:" << std::endl;
    std::string str1 = "Hello";
    std::string str2 = " C++";
    std::string concatenated = Task114::concatenateStrings(str1, str2);
    std::cout << "String 1: " << str1 << std::endl;
    std::cout << "String 2: " << str2 << std::endl;
    std::cout << "Concatenated: " << concatenated << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Reverse String
    std::cout << "Test Case 3 - Reverse String:" << std::endl;
    std::string toReverse = "Programming";
    std::string reversed = Task114::reverseString(toReverse);
    std::cout << "Original: " << toReverse << std::endl;
    std::cout << "Reversed: " << reversed << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Case Conversion
    std::cout << "Test Case 4 - Case Conversion:" << std::endl;
    std::string mixedCase = "HeLLo WoRLd";
    std::string upper = Task114::toUpperCaseSafe(mixedCase);
    std::string lower = Task114::toLowerCaseSafe(mixedCase);
    std::cout << "Original: " << mixedCase << std::endl;
    std::cout << "Uppercase: " << upper << std::endl;
    std::cout << "Lowercase: " << lower << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Substring and Replace
    std::cout << "Test Case 5 - Substring and Replace:" << std::endl;
    std::string text = "The quick brown fox jumps over the lazy dog";
    std::string substring = Task114::getSubstringSafe(text, 4, 9);
    std::string replaced = Task114::replaceSubstring(text, "fox", "cat");
    std::cout << "Original: " << text << std::endl;
    std::cout << "Substring (4-9): " << substring << std::endl;
    std::cout << "Replaced 'fox' with 'cat': " << replaced << std::endl;
    
    return 0;
}
