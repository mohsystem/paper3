// C++ implementation
// Defensive checks, simple arithmetic, no dynamic memory or unsafe operations.
#include <iostream>
#include <vector>
#include <stdexcept>

char findMissingLetter(const std::vector<char>& arr) {
    if (arr.size() < 2) {
        throw std::invalid_argument("Invalid input: array must have at least 2 characters.");
    }
    for (size_t i = 1; i < arr.size(); ++i) {
        int diff = static_cast<int>(arr[i]) - static_cast<int>(arr[i - 1]);
        if (diff != 1) {
            return static_cast<char>(arr[i - 1] + 1);
        }
    }
    throw std::runtime_error("No missing letter found.");
}

int main() {
    std::vector<std::vector<char>> tests = {
        {'a','b','c','d','f'},
        {'O','Q','R','S'},
        {'w','x','z'},
        {'A','B','C','E'},
        {'m','n','p','q','r'}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            char result = findMissingLetter(tests[i]);
            std::cout << "Test " << (i + 1) << " missing letter: " << result << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i + 1) << " error: " << ex.what() << "\n";
        }
    }
    return 0;
}