#include <iostream>
#include <vector>
#include <stdexcept>
#include <string>

class Task2 {
public:
    static std::string createPhoneNumber(const std::vector<int>& numbers) {
        if (numbers.size() != 10) {
            throw std::invalid_argument("Input array must contain exactly 10 integers.");
        }
        for (int n : numbers) {
            if (n < 0 || n > 9) {
                throw std::invalid_argument("All numbers must be in the range 0 to 9.");
            }
        }
        std::string s;
        s.reserve(14);
        s.push_back('(');
        s.push_back(static_cast<char>('0' + numbers[0]));
        s.push_back(static_cast<char>('0' + numbers[1]));
        s.push_back(static_cast<char>('0' + numbers[2]));
        s.append(") ");
        s.push_back(static_cast<char>('0' + numbers[3]));
        s.push_back(static_cast<char>('0' + numbers[4]));
        s.push_back(static_cast<char>('0' + numbers[5]));
        s.push_back('-');
        s.push_back(static_cast<char>('0' + numbers[6]));
        s.push_back(static_cast<char>('0' + numbers[7]));
        s.push_back(static_cast<char>('0' + numbers[8]));
        s.push_back(static_cast<char>('0' + numbers[9]));
        return s;
    }
};

int main() {
    std::vector<std::vector<int>> tests = {
        {1,2,3,4,5,6,7,8,9,0},
        {0,0,0,0,0,0,0,0,0,0},
        {9,8,7,6,5,4,3,2,1,0},
        {1,1,1,1,1,1,1,1,1,1},
        {1,2,3,4,-5,6,7,8,9,0} // invalid
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            std::string result = Task2::createPhoneNumber(tests[i]);
            std::cout << "Test " << (i+1) << " : " << result << std::endl;
        } catch (const std::exception& ex) {
            std::cout << "Test " << (i+1) << " : ERROR - " << ex.what() << std::endl;
        }
    }
    return 0;
}