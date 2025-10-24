
#include <iostream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <cctype>

bool isIsogram(const std::string& str) {
    if (str.empty()) {
        return true;
    }
    
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    
    std::unordered_set<char> charSet;
    
    for (char c : lowerStr) {
        if (charSet.find(c) != charSet.end()) {
            return false;
        }
        charSet.insert(c);
    }
    
    return true;
}

int main() {
    // Test cases
    std::cout << "Test 1: 'Dermatoglyphics' -> " << (isIsogram("Dermatoglyphics") ? "true" : "false") << std::endl;
    std::cout << "Test 2: 'aba' -> " << (isIsogram("aba") ? "true" : "false") << std::endl;
    std::cout << "Test 3: 'moOse' -> " << (isIsogram("moOse") ? "true" : "false") << std::endl;
    std::cout << "Test 4: '' -> " << (isIsogram("") ? "true" : "false") << std::endl;
    std::cout << "Test 5: 'isogram' -> " << (isIsogram("isogram") ? "true" : "false") << std::endl;
    
    return 0;
}
