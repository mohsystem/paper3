#include <iostream>
#include <string>
#include <cmath>
#include <vector>

class Task34 {
public:
    /**
     * Checks if a given number is a Narcissistic Number (Armstrong Number).
     *
     * @param number The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    static bool isNarcissistic(int number) {
        if (number <= 0) {
            return false;
        }

        std::string numberStr = std::to_string(number);
        int digits = numberStr.length();
        long long sum = 0;
        int temp = number;

        while (temp > 0) {
            int digit = temp % 10;
            sum += static_cast<long long>(pow(digit, digits));
            temp /= 10;
        }

        return sum == number;
    }
};

int main() {
    std::vector<int> testCases = {153, 1652, 1, 371, 9474};
    std::cout << std::boolalpha; // Print booleans as "true" or "false"
    for (int testCase : testCases) {
        std::cout << Task34::isNarcissistic(testCase) << std::endl;
    }
    return 0;
}