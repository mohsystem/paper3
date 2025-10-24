#include <iostream>
#include <string>
#include <cctype>

/**
 * Checks if a string has the same amount of 'x's and 'o's, case-insensitively.
 *
 * @param str The string to check. The string can contain any character.
 * @return true if the count of 'x's and 'o's is equal, false otherwise.
 */
bool XO(const std::string& str) {
    int xCount = 0;
    int oCount = 0;

    for (char ch : str) {
        // Use static_cast to unsigned char for safe conversion in tolower
        char lower_ch = std::tolower(static_cast<unsigned char>(ch));
        if (lower_ch == 'x') {
            xCount++;
        } else if (lower_ch == 'o') {
            oCount++;
        }
    }
    
    return xCount == oCount;
}

int main() {
    // std::boolalpha prints "true" or "false" for bool values
    std::cout << std::boolalpha;

    // Test cases
    std::cout << "XO(\"ooxx\") => " << XO("ooxx") << std::endl;          // Expected: true
    std::cout << "XO(\"xooxx\") => " << XO("xooxx") << std::endl;        // Expected: false
    std::cout << "XO(\"ooxXm\") => " << XO("ooxXm") << std::endl;        // Expected: true
    std::cout << "XO(\"zpzpzpp\") => " << XO("zpzpzpp") << std::endl;    // Expected: true
    std::cout << "XO(\"zzoo\") => " << XO("zzoo") << std::endl;          // Expected: false
    
    return 0;
}