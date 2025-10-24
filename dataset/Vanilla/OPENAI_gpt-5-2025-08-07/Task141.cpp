#include <iostream>
#include <string>
#include <vector>

std::string reverseString(const std::string& s) {
    return std::string(s.rbegin(), s.rend());
}

int main() {
    std::vector<std::string> tests = {"", "a", "abcde", "Hello, World!", "12345 racecar"};
    for (const auto& t : tests) {
        std::cout << reverseString(t) << std::endl;
    }
    return 0;
}