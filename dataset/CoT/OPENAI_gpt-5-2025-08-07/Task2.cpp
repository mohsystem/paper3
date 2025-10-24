// Chain-of-Through process in code generation:
// 1) Problem understanding: Convert 10 digits (0-9) into "(XXX) XXX-XXXX".
// 2) Security requirements: Validate input size and range, avoid undefined behavior.
// 3) Secure coding generation: Use bounds-checked logic and safe construction.
// 4) Code review: No unchecked indexing beyond validated range, no leaks.
// 5) Secure code output: Final function with exception handling and tests.

#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

std::string createPhoneNumber(const std::vector<int>& numbers) {
    if (numbers.size() != 10) {
        throw std::invalid_argument("Input must contain exactly 10 integers.");
    }
    for (int n : numbers) {
        if (n < 0 || n > 9) {
            throw std::invalid_argument("All integers must be in the range 0-9.");
        }
    }
    std::string r;
    r.reserve(14);
    r.push_back('(');
    r.push_back(static_cast<char>('0' + numbers[0]));
    r.push_back(static_cast<char>('0' + numbers[1]));
    r.push_back(static_cast<char>('0' + numbers[2]));
    r.push_back(')');
    r.push_back(' ');
    r.push_back(static_cast<char>('0' + numbers[3]));
    r.push_back(static_cast<char>('0' + numbers[4]));
    r.push_back(static_cast<char>('0' + numbers[5]));
    r.push_back('-');
    r.push_back(static_cast<char>('0' + numbers[6]));
    r.push_back(static_cast<char>('0' + numbers[7]));
    r.push_back(static_cast<char>('0' + numbers[8]));
    r.push_back(static_cast<char>('0' + numbers[9]));
    return r;
}

int main() {
    // 5 test cases
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,5,6,7,8,9,0},
        {0,1,2,3,4,5,6,7,8,9},
        {0,1,2,3,4,5,6,7,8},          // invalid length
        {-1,1,2,3,4,5,6,7,8,9},       // invalid value negative
        {0,1,2,3,4,5,6,7,8,10}        // invalid value > 9
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            std::cout << "Test " << (i+1) << ": " << createPhoneNumber(tests[i]) << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test " << (i+1) << " Error: " << e.what() << "\n";
        }
    }
    return 0;
}