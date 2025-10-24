
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task168 {
public:
    static std::string tweakLetters(const std::string& str, const std::vector<int>& tweaks) {
        // Input validation
        if (str.empty()) {
            return "";
        }
        
        // Ensure arrays have matching lengths
        size_t length = std::min(str.length(), tweaks.size());
        std::string result;
        result.reserve(length);
        
        for (size_t i = 0; i < length; i++) {
            char c = str[i];
            int tweak = tweaks[i];
            
            // Validate tweak value is within safe range
            if (tweak < -25 || tweak > 25) {
                result += c;
                continue;
            }
            
            // Only process lowercase letters
            if (c >= 'a' && c <= 'z') {
                // Calculate new character with wrapping
                int offset = c - 'a';
                int newOffset = (offset + tweak) % 26;
                // Handle negative modulo
                if (newOffset < 0) {
                    newOffset += 26;
                }
                result += static_cast<char>('a' + newOffset);
            } else {
                // Non-lowercase letters remain unchanged
                result += c;
            }
        }
        
        return result;
    }
};

int main() {
    // Test case 1
    std::cout << Task168::tweakLetters("apple", {0, 1, -1, 0, -1}) << std::endl;
    // Expected: "aqold"
    
    // Test case 2
    std::cout << Task168::tweakLetters("many", {0, 0, 0, -1}) << std::endl;
    // Expected: "manx"
    
    // Test case 3
    std::cout << Task168::tweakLetters("rhino", {1, 1, 1, 1, 1}) << std::endl;
    // Expected: "sijop"
    
    // Test case 4
    std::cout << Task168::tweakLetters("zebra", {1, -1, 0, 1, -1}) << std::endl;
    // Expected: "aabrz"
    
    // Test case 5
    std::cout << Task168::tweakLetters("code", {-1, 1, -1, 1}) << std::endl;
    // Expected: "bpcf"
    
    return 0;
}
