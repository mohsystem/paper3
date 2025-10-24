#include <iostream>
#include <vector>

class Task8 {
public:
    /**
     * @brief Finds the missing letter in a vector of consecutive, increasing letters.
     * The vector will always be valid and have exactly one letter missing.
     * 
     * @param arr A constant reference to a vector of characters.
     * @return The missing character.
     */
    static char findMissingLetter(const std::vector<char>& arr) {
        // Since the array length is at least 2, arr[i-1] is always a safe access.
        for (size_t i = 1; i < arr.size(); ++i) {
            // Check if the current character's ASCII value is not one greater than the previous one.
            if (arr[i] != arr[i-1] + 1) {
                // If not, the missing character is the previous character + 1.
                return static_cast<char>(arr[i-1] + 1);
            }
        }
        // This part is unreachable given the problem constraints.
        // It is included to satisfy compiler warnings about control reaching the end of a non-void function.
        return ' ';
    }
};

int main() {
    // Test Case 1
    std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    std::cout << "Test 1: ['a','b','c','d','f'] -> Expected: e, Got: " << Task8::findMissingLetter(test1) << std::endl;

    // Test Case 2
    std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
    std::cout << "Test 2: ['O','Q','R','S'] -> Expected: P, Got: " << Task8::findMissingLetter(test2) << std::endl;
    
    // Test Case 3
    std::vector<char> test3 = {'x', 'z'};
    std::cout << "Test 3: ['x','z'] -> Expected: y, Got: " << Task8::findMissingLetter(test3) << std::endl;

    // Test Case 4
    std::vector<char> test4 = {'A', 'B', 'D'};
    std::cout << "Test 4: ['A','B','D'] -> Expected: C, Got: " << Task8::findMissingLetter(test4) << std::endl;

    // Test Case 5
    std::vector<char> test5 = {'m', 'n', 'o', 'q'};
    std::cout << "Test 5: ['m','n','o','q'] -> Expected: p, Got: " << Task8::findMissingLetter(test5) << std::endl;

    return 0;
}