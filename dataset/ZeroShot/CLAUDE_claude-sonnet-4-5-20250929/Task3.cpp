
#include <iostream>
#include <string>
#include <set>
#include <cctype>

bool isPangram(const std::string& sentence) {
    if (sentence.empty()) {
        return false;
    }
    
    std::set<char> letters;
    
    for (char c : sentence) {
        if (std::isalpha(c)) {
            letters.insert(std::tolower(c));
        }
    }
    
    return letters.size() == 26;
}

int main() {
    // Test cases
    std::cout << "Test 1: " << (isPangram("The quick brown fox jumps over the lazy dog") ? "true" : "false") << std::endl;
    std::cout << "Test 2: " << (isPangram("Hello World") ? "true" : "false") << std::endl;
    std::cout << "Test 3: " << (isPangram("abcdefghijklmnopqrstuvwxyz") ? "true" : "false") << std::endl;
    std::cout << "Test 4: " << (isPangram("Pack my box with five dozen liquor jugs") ? "true" : "false") << std::endl;
    std::cout << "Test 5: " << (isPangram("This is not a pangram sentence") ? "true" : "false") << std::endl;
    
    return 0;
}
