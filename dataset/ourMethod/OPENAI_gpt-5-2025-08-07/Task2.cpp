#include <iostream>
#include <vector>
#include <string>

static bool validateDigits(const std::vector<int>& digits) {
    if (digits.size() != 10) return false;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (digits[i] < 0 || digits[i] > 9) return false;
    }
    return true;
}

std::string createPhoneNumber(const std::vector<int>& digits) {
    if (!validateDigits(digits)) {
        return std::string(); // return empty string on invalid input
    }
    std::string s;
    s.reserve(14);
    s.push_back('(');
    s.push_back(static_cast<char>('0' + digits[0]));
    s.push_back(static_cast<char>('0' + digits[1]));
    s.push_back(static_cast<char>('0' + digits[2]));
    s.push_back(')');
    s.push_back(' ');
    s.push_back(static_cast<char>('0' + digits[3]));
    s.push_back(static_cast<char>('0' + digits[4]));
    s.push_back(static_cast<char>('0' + digits[5]));
    s.push_back('-');
    s.push_back(static_cast<char>('0' + digits[6]));
    s.push_back(static_cast<char>('0' + digits[7]));
    s.push_back(static_cast<char>('0' + digits[8]));
    s.push_back(static_cast<char>('0' + digits[9]));
    return s;
}

int main() {
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,5,6,7,8,9,0},
        {0,0,0,0,0,0,0,0,0,0},
        {9,8,7,6,5,4,3,2,1,0},
        {5,5,5,1,2,1,2,3,4,5},
        {2,1,2,5,5,5,0,1,0,0}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string result = createPhoneNumber(tests[i]);
        if (result.empty()) {
            std::cout << "Test " << (i + 1) << ": Invalid input\n";
        } else {
            std::cout << "Test " << (i + 1) << ": " << result << "\n";
        }
    }
    return 0;
}