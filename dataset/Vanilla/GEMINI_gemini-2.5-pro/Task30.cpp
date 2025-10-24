#include <iostream>
#include <string>
#include <set>
#include <algorithm>

class Task30 {
public:
    static std::string longest(const std::string& s1, const std::string& s2) {
        // A set automatically stores unique elements in sorted order.
        std::set<char> chars;
        
        // Insert all characters from s1 and s2 into the set.
        chars.insert(s1.begin(), s1.end());
        chars.insert(s2.begin(), s2.end());
        
        // Construct a new string from the characters in the set.
        return std::string(chars.begin(), chars.end());
    }
};

int main() {
    // Test case 1
    std::cout << "Test 1: " << Task30::longest("xyaabbbccccdefww", "xxxxyyyyabklmopq") << std::endl;

    // Test case 2
    std::cout << "Test 2: " << Task30::longest("abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz") << std::endl;

    // Test case 3
    std::cout << "Test 3: " << Task30::longest("aretheyhere", "yestheyarehere") << std::endl;

    // Test case 4
    std::cout << "Test 4: " << Task30::longest("loopingisfunbutdangerous", "lessdangerousthancoding") << std::endl;

    // Test case 5
    std::cout << "Test 5: " << Task30::longest("inmanylanguages", "theresapairoffunctions") << std::endl;

    return 0;
}