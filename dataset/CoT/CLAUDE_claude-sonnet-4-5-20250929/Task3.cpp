
#include <iostream>
#include <string>
#include <unordered_set>
#include <cctype>

bool isPangram(const std::string& text) {
    if (text.empty()) {
        return false;
    }
    
    std::unordered_set<char> alphabetSet;
    
    for (char c : text) {
        char lower = std::tolower(static_cast<unsigned char>(c));
        if (lower >= 'a' && lower <= 'z') {
            alphabetSet.insert(lower);
        }
    }
    
    return alphabetSet.size() == 26;
}

int main() {
    // Test case 1: Classic pangram
    std::cout << "Test 1: " << (isPangram("The quick brown fox jumps over the lazy dog") ? "true" : "false") << std::endl;
    
    // Test case 2: Pangram with numbers and punctuation
    std::cout << "Test 2: " << (isPangram("Pack my box with five dozen liquor jugs!123") ? "true" : "false") << std::endl;
    
    // Test case 3: Not a pangram
    std::cout << "Test 3: " << (isPangram("Hello World") ? "true" : "false") << std::endl;
    
    // Test case 4: Empty string
    std::cout << "Test 4: " << (isPangram("") ? "true" : "false") << std::endl;
    
    // Test case 5: All letters with special characters
    std::cout << "Test 5: " << (isPangram("abcdefghijklmnopqrstuvwxyz!!!") ? "true" : "false") << std::endl;
    
    return 0;
}
