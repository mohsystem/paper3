
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

std::string reverseString(const std::string& str) {
    std::string reversed = str;
    std::reverse(reversed.begin(), reversed.end());
    return reversed;
}

std::string reverseWords(const std::string& input) {
    std::istringstream iss(input);
    std::string word;
    std::string result;
    bool first = true;
    
    while (iss >> word) {
        if (!first) {
            result += " ";
        }
        first = false;
        
        if (word.length() >= 5) {
            result += reverseString(word);
        } else {
            result += word;
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    std::cout << reverseWords("Hey fellow warriors") << std::endl;
    
    // Test case 2
    std::cout << reverseWords("This is a test") << std::endl;
    
    // Test case 3
    std::cout << reverseWords("This is another test") << std::endl;
    
    // Test case 4
    std::cout << reverseWords("Hello") << std::endl;
    
    // Test case 5
    std::cout << reverseWords("Hi") << std::endl;
    
    return 0;
}
