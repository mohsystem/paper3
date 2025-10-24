#include <iostream>
#include <string>
#include <cmath>

class Task20 {
public:
    static int digPow(int n, int p) {
        std::string s = std::to_string(n);
        long long sum = 0;
        for (size_t i = 0; i < s.length(); ++i) {
            int digit = s[i] - '0';
            sum += pow(digit, p + i);
        }

        if (sum % n == 0) {
            return static_cast<int>(sum / n);
        } else {
            return -1;
        }
    }
};

int main() {
    std::cout << "n=89, p=1 -> " << Task20::digPow(89, 1) << std::endl;
    std::cout << "n=92, p=1 -> " << Task20::digPow(92, 1) << std::endl;
    std::cout << "n=695, p=2 -> " << Task20::digPow(695, 2) << std::endl;
    std::cout << "n=46288, p=3 -> " << Task20::digPow(46288, 3) << std::endl;
    std::cout << "n=114, p=3 -> " << Task20::digPow(114, 3) << std::endl;
    return 0;
}