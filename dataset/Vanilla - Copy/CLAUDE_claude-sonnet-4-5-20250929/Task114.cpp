
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

class Task114 {
public:
    // Copy string
    static std::string copyString(const std::string& source) {
        return std::string(source);
    }
    
    // Concatenate two strings
    static std::string concatenateStrings(const std::string& str1, const std::string& str2) {
        return str1 + str2;
    }
    
    // Reverse a string
    static std::string reverseString(const std::string& str) {
        std::string reversed = str;
        std::reverse(reversed.begin(), reversed.end());
        return reversed;
    }
    
    // Convert to uppercase
    static std::string toUpperCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    // Convert to lowercase
    static std::string toLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }
    
    // Get substring
    static std::string getSubstring(const std::string& str, int start, int end) {
        if (start < 0 || end > str.length() || start > end) {
            return "";
        }
        return str.substr(start, end - start);
    }
    
    // Count character occurrences
    static int countCharacter(const std::string& str, char ch) {
        int count = 0;
        for (char c : str) {
            if (c == ch) {
                count++;
            }
        }
        return count;
    }
    
    // Replace character
    static std::string replaceCharacter(const std::string& str, char oldChar, char newChar) {
        std::string result = str;
        std::replace(result.begin(), result.end(), oldChar, newChar);
        return result;
    }
    
    // Find string length
    static int getLength(const std::string& str) {
        return str.length();
    }
    
    // Check if string is palindrome
    static bool isPalindrome(const std::string& str) {
        return str == reverseString(str);
    }
};

int main() {
    std::cout << "=== Test Case 1: Copy and Concatenate ===" << std::endl;
    std::string original = "Hello";
    std::string copied = Task114::copyString(original);
    std::cout << "Original: " << original << std::endl;
    std::cout << "Copied: " << copied << std::endl;
    std::string concatenated = Task114::concatenateStrings("Hello", " World");
    std::cout << "Concatenated: " << concatenated << std::endl;
    
    std::cout << "\\n=== Test Case 2: Reverse String ===" << std::endl;
    std::string str2 = "Programming";
    std::cout << "Original: " << str2 << std::endl;
    std::cout << "Reversed: " << Task114::reverseString(str2) << std::endl;
    
    std::cout << "\\n=== Test Case 3: Case Conversion ===" << std::endl;
    std::string str3 = "Java Python CPP";
    std::cout << "Original: " << str3 << std::endl;
    std::cout << "Uppercase: " << Task114::toUpperCase(str3) << std::endl;
    std::cout << "Lowercase: " << Task114::toLowerCase(str3) << std::endl;
    
    std::cout << "\\n=== Test Case 4: Substring and Character Operations ===" << std::endl;
    std::string str4 = "Hello World";
    std::cout << "Original: " << str4 << std::endl;
    std::cout << "Substring (0, 5): " << Task114::getSubstring(str4, 0, 5) << std::endl;
    std::cout << "Count 'l': " << Task114::countCharacter(str4, 'l') << std::endl;
    std::cout << "Replace 'o' with '0': " << Task114::replaceCharacter(str4, 'o', '0') << std::endl;
    std::cout << "Length: " << Task114::getLength(str4) << std::endl;
    
    std::cout << "\\n=== Test Case 5: Palindrome Check ===" << std::endl;
    std::string str5a = "radar";
    std::string str5b = "hello";
    std::cout << str5a << " is palindrome: " << (Task114::isPalindrome(str5a) ? "true" : "false") << std::endl;
    std::cout << str5b << " is palindrome: " << (Task114::isPalindrome(str5b) ? "true" : "false") << std::endl;
    
    return 0;
}
