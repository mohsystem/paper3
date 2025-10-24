
#include <iostream>
#include <string>
#include <cctype>

std::string accum(const std::string& s) {
    if (s.empty()) {
        return "";
    }
    
    std::string result;
    
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        
        // Validate input - only letters allowed
        if (!std::isalpha(static_cast<unsigned char>(c))) {
            continue;
        }
        
        // Add separator if not first element
        if (!result.empty()) {
            result += "-";
        }
        
        // First character uppercase
        result += std::toupper(static_cast<unsigned char>(c));
        
        // Rest lowercase, repeated i times
        for (size_t j = 0; j < i; j++) {
            result += std::tolower(static_cast<unsigned char>(c));
        }
    }
    
    return result;
}

int main() {
    // Test case 1
    std::cout << "Test 1: " << accum("abcd") << std::endl;
    // Expected: A-Bb-Ccc-Dddd
    
    // Test case 2
    std::cout << "Test 2: " << accum("RqaEzty") << std::endl;
    // Expected: R-Qq-Aaa-Eeee-Zzzzz-Tttttt-Yyyyyyy
    
    // Test case 3
    std::cout << "Test 3: " << accum("cwAt") << std::endl;
    // Expected: C-Ww-Aaa-Tttt
    
    // Test case 4
    std::cout << "Test 4: " << accum("ZpglnRxqenU") << std::endl;
    // Expected: Z-Pp-Ggg-Llll-Nnnnn-Rrrrrr-Xxxxxxx-Qqqqqqqq-Eeeeeeeee-Nnnnnnnnnn-Uuuuuuuuuuuu
    
    // Test case 5
    std::cout << "Test 5: " << accum("a") << std::endl;
    // Expected: A
    
    return 0;
}
