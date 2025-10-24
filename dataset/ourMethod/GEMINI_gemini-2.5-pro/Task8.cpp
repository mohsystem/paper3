#include <iostream>
#include <vector>

/**
 * @brief Finds the missing letter in an array of consecutive letters.
 *
 * @param arr A vector of characters representing a sequence of consecutive,
 *            increasing letters with exactly one letter missing. The vector
 *            will always have at least 2 elements.
 * @return The missing character.
 */
char findMissingLetter(const std::vector<char>& arr) {
    // The prompt guarantees arr.size() >= 2.
    // Loop starts from the second element to compare with the previous one.
    for (size_t i = 1; i < arr.size(); ++i) {
        // If the ASCII value of the current char is not one greater than the previous,
        // we've found the gap. The missing letter is the character after the previous one.
        if (static_cast<unsigned char>(arr[i]) != static_cast<unsigned char>(arr[i-1]) + 1) {
            return static_cast<char>(arr[i-1] + 1);
        }
    }
    // This part of the code should be unreachable given the problem constraints
    // (always exactly one letter missing). Return a null character as a fallback.
    return '\0';
}

int main() {
    // Test Case 1
    std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    std::cout << "Test Case 1: ['a','b','c','d','f'] -> '" << findMissingLetter(test1) << "'" << std::endl;

    // Test Case 2
    std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
    std::cout << "Test Case 2: ['O','Q','R','S'] -> '" << findMissingLetter(test2) << "'" << std::endl;

    // Test Case 3
    std::vector<char> test3 = {'x', 'z'};
    std::cout << "Test Case 3: ['x', 'z'] -> '" << findMissingLetter(test3) << "'" << std::endl;

    // Test Case 4
    std::vector<char> test4 = {'F', 'G', 'H', 'J'};
    std::cout << "Test Case 4: ['F', 'G', 'H', 'J'] -> '" << findMissingLetter(test4) << "'" << std::endl;

    // Test Case 5
    std::vector<char> test5 = {'m', 'n', 'o', 'q', 'r'};
    std::cout << "Test Case 5: ['m', 'n', 'o', 'q', 'r'] -> '" << findMissingLetter(test5) << "'" << std::endl;

    return 0;
}