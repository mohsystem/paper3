#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstdio>

std::string createPhoneNumber(const std::vector<int>& numbers) {
    if (numbers.size() != 10) {
        throw std::invalid_argument("Input must be 10 integers");
    }
    for (int n : numbers) {
        if (n < 0 || n > 9) {
            throw std::invalid_argument("Digits must be between 0 and 9");
        }
    }
    char buf[15];
    std::snprintf(buf, sizeof(buf), "(%d%d%d) %d%d%d-%d%d%d%d",
                  numbers[0], numbers[1], numbers[2],
                  numbers[3], numbers[4], numbers[5],
                  numbers[6], numbers[7], numbers[8], numbers[9]);
    return std::string(buf);
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,5,6,7,8,9,0},
        {0,0,0,0,0,0,0,0,0,0},
        {9,8,7,6,5,4,3,2,1,0},
        {5,5,5,1,2,3,4,5,6,7},
        {1,2,3,1,2,3,1,2,3,4}
    };
    for (const auto& t : tests) {
        std::cout << createPhoneNumber(t) << std::endl;
    }
    return 0;
}