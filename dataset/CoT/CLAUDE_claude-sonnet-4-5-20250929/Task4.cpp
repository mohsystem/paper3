
#include <iostream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <algorithm>

bool isIsogram(const std::string& str) {
    std::string lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
    
    std::unordered_set<char> seenChars;
    
    for (size_t i = 0; i < lowerStr.length(); i++) {
        char c = lowerStr[i];
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (seenChars.find(c) != seenChars.end()) {
                return false;
            }
            seenChars.insert(c);
        }
    }
    
    return true;
}

int main() {
    // Test case 1
    std::cout << "Test 1: 'Dermatoglyphics' -> " << (isIsogram("Dermatoglyphics") ? "true" : "false") << std::endl;
    
    // Test case 2
    std::cout << "Test 2: 'aba' -> " << (isIsogram("aba") ? "true" : "false") << std::endl;
    
    // Test case 3
    std::cout << "Test 3: 'moOse' -> " << (isIsogram("moOse") ? "true" : "false") << std::endl;
    
    // Test case 4
    std::cout << "Test 4: '' -> " << (isIsogram("") ? "true" : "false") << std::endl;
    
    // Test case 5
    std::cout << "Test 5: 'isogram' -> " << (isIsogram("isogram") ? "true" : "false") << std::endl;
    
    return 0;
}
