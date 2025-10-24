#include <iostream>
#include <string>
#include <cmath>

class Task20 {
public:
    /**
     * Finds a positive integer k, if it exists, such that the sum of the digits of n
     * raised to consecutive powers starting from p is equal to k * n.
     *
     * @param n a positive integer.
     * @param p a positive integer.
     * @return a positive integer k, or -1 if no such k exists.
     */
    static long long digPow(int n, int p) {
        std::string nStr = std::to_string(n);
        long long sum = 0;
        for (char c : nStr) {
            // Convert character to digit and add its power to sum
            sum += pow(c - '0', p++);
        }

        // Check if the sum is a multiple of n
        if (sum % n == 0) {
            return sum / n;
        } else {
            return -1;
        }
    }
};

int main() {
    std::cout << "n = 89, p = 1 --> " << Task20::digPow(89, 1) << std::endl;
    std::cout << "n = 92, p = 1 --> " << Task20::digPow(92, 1) << std::endl;
    std::cout << "n = 695, p = 2 --> " << Task20::digPow(695, 2) << std::endl;
    std::cout << "n = 46288, p = 3 --> " << Task20::digPow(46288, 3) << std::endl;
    std::cout << "n = 135, p = 1 --> " << Task20::digPow(135, 1) << std::endl;
    return 0;
}