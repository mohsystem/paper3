#include <iostream>
#include <string>
#include <vector>

/**
 * Checks if the first string ends with the second string.
 * This implementation is secure. It first checks if the ending string
 * is longer than the main string, preventing out-of-bounds access.
 * It then uses std::string::compare, a safe standard library function,
 * to compare the relevant part of the main string with the ending string.
 *
 * @param str The main string.
 * @param ending The string to check for at the end of str.
 * @return true if str ends with ending, false otherwise.
 */
bool solution(const std::string& str, const std::string& ending) {
    if (ending.length() > str.length()) {
        return false;
    }
    // Compare the end of `str` with `ending`
    // str.length() - ending.length() is the starting position in `str`
    // ending.length() is the number of characters to compare
    return str.compare(str.length() - ending.length(), ending.length(), ending) == 0;
}

int main() {
    // Test cases
    std::cout << std::boolalpha; // Print booleans as true/false
    std::cout << "('abc', 'bc') -> " << solution("abc", "bc") << std::endl; // true
    std::cout << "('abc', 'd') -> " << solution("abc", "d") << std::endl; // false
    std::cout << "('sushi', 'shi') -> " << solution("sushi", "shi") << std::endl; // true
    std::cout << "('samurai', 'ra') -> " << solution("samurai", "ra") << std::endl; // false
    std::cout << "('abc', '') -> " << solution("abc", "") << std::endl; // true

    return 0;
}