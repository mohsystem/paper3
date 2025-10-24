#include <iostream>
#include <vector>

char findMissingLetter(const std::vector<char>& arr) {
    for (size_t i = 0; i < arr.size() - 1; ++i) {
        // If the difference between ASCII values is not 1, a letter is missing.
        if (arr[i+1] - arr[i] > 1) {
            // The missing letter is the current letter's ASCII value + 1
            return static_cast<char>(arr[i] + 1);
        }
    }
    // This part should not be reached given the problem constraints.
    return ' ';
}

int main() {
    // Test Case 1
    std::vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    std::cout << "['a','b','c','d','f'] -> " << findMissingLetter(test1) << std::endl;

    // Test Case 2
    std::vector<char> test2 = {'O', 'Q', 'R', 'S'};
    std::cout << "['O','Q','R','S'] -> " << findMissingLetter(test2) << std::endl;

    // Test Case 3
    std::vector<char> test3 = {'x', 'z'};
    std::cout << "['x','z'] -> " << findMissingLetter(test3) << std::endl;

    // Test Case 4
    std::vector<char> test4 = {'F', 'G', 'H', 'J'};
    std::cout << "['F','G','H','J'] -> " << findMissingLetter(test4) << std::endl;

    // Test Case 5
    std::vector<char> test5 = {'m', 'n', 'o', 'q', 'r'};
    std::cout << "['m','n','o','q','r'] -> " << findMissingLetter(test5) << std::endl;

    return 0;
}