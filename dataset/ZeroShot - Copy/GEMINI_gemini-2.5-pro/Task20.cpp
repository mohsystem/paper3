#include <iostream>
#include <string>
#include <cmath>
#include <vector>

class Task20 {
public:
    /**
     * @brief Finds a positive integer k, if it exists, such that the sum of the digits of n
     * raised to consecutive powers starting from p is equal to k * n.
     * 
     * @param n A positive integer.
     * @param p A positive integer.
     * @return The integer k if found, otherwise -1.
     */
    static long long digPow(int n, int p) {
        std::string s = std::to_string(n);
        long long sum = 0;
        for (size_t i = 0; i < s.length(); ++i) {
            int digit = s[i] - '0';
            sum += static_cast<long long>(round(pow(digit, p + i)));
        }

        if (n > 0 && sum % n == 0) {
            return sum / n;
        } else {
            return -1;
        }
    }
};

int main() {
    // Test cases
    std::cout << "n=89, p=1 -> " << Task20::digPow(89, 1) << std::endl;
    std::cout << "n=92, p=1 -> " << Task20::digPow(92, 1) << std::endl;
    std::cout << "n=695, p=2 -> " << Task20::digPow(695, 2) << std::endl;
    std::cout << "n=46288, p=3 -> " << Task20::digPow(46288, 3) << std::endl;
    std::cout << "n=135, p=1 -> " << Task20::digPow(135, 1) << std::endl;
    return 0;
}