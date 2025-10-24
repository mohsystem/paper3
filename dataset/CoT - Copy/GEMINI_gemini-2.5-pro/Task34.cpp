#include <iostream>
#include <string>
#include <cmath>
#include <vector>

class Task34 {
public:
    /**
     * @brief Checks if a number is a Narcissistic Number (Armstrong Number).
     * 
     * @param value The positive integer to check.
     * @return true if the number is narcissistic, false otherwise.
     */
    static bool narcissistic(int value) {
        // The prompt guarantees positive non-zero integers.
        if (value <= 0) {
            return false;
        }

        std::string sValue = std::to_string(value);
        int numDigits = sValue.length();
        long long sum = 0; // Use long long to avoid overflow

        int temp = value;
        while (temp > 0) {
            int digit = temp % 10;
            sum += pow(digit, numDigits);
            temp /= 10;
        }

        return sum == value;
    }
};

int main() {
    std::vector<int> test_cases = {7, 153, 371, 1634, 1652};
    std::cout << std::boolalpha; // Print "true" or "false" instead of 1 or 0
    for (int test_case : test_cases) {
        std::cout << "Is " << test_case << " narcissistic? " 
                  << Task34::narcissistic(test_case) 
                  << std::endl;
    }
    return 0;
}