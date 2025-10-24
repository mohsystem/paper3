
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

std::string reverseWords(const std::string& str) {
    if (str.empty()) {
        return str;
    }
    
    std::istringstream iss(str);
    std::string word;
    std::vector<std::string> words;
    
    while (iss >> word) {
        words.push_back(word);
    }
    
    std::string result;
    for (size_t i = 0; i < words.size(); i++) {
        if (words[i].length() >= 5) {
            std::string reversed = words[i];
            std::reverse(reversed.begin(), reversed.end());
            result += reversed;
        } else {
            result += words[i];
        }
        
        if (i < words.size() - 1) {
            result += " ";
        }
    }
    
    return result;
}

int main() {
    // Test cases
    std::cout << "Test 1: " << reverseWords("Hey fellow warriors") << std::endl;
    std::cout << "Expected: Hey wollef sroirraw" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 2: " << reverseWords("This is a test") << std::endl;
    std::cout << "Expected: This is a test" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 3: " << reverseWords("This is another test") << std::endl;
    std::cout << "Expected: This is rehtona test" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 4: " << reverseWords("abcde") << std::endl;
    std::cout << "Expected: edcba" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Test 5: " << reverseWords("a b c d") << std::endl;
    std::cout << "Expected: a b c d" << std::endl;
    
    return 0;
}
