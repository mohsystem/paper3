#include <iostream>
#include <vector>

char findMissingLetter(const std::vector<char>& array) {
    for (size_t i = 1; i < array.size(); ++i) {
        if (array[i] != static_cast<char>(array[i - 1] + 1)) {
            return static_cast<char>(array[i - 1] + 1);
        }
    }
    return static_cast<char>(array.back() + 1);
}

int main() {
    std::vector<std::vector<char>> tests = {
        {'a','b','c','d','f'},
        {'O','Q','R','S'},
        {'b','c','d','e','g'},
        {'A','B','D'},
        {'m','n','p','q','r'}
    };
    for (const auto& t : tests) {
        std::cout << findMissingLetter(t) << std::endl;
        }
    return 0;
}