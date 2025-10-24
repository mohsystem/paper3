#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task60 {
public:
    /**
     * Converts a number to its expanded form string.
     * For example: 70304 becomes "70000 + 300 + 4"
     * @param num The positive integer to convert (using long long for wider range).
     * @return The string representing the expanded form.
     */
    static std::string expandedForm(long long num) {
        std::vector<std::string> parts;
        long long place = 1;

        while (num > 0) {
            long long rem = num % 10;
            if (rem != 0) {
                parts.push_back(std::to_string(rem * place));
            }
            num /= 10;
            place *= 10;
        }
        
        std::string result = "";
        // The parts are generated from right-to-left, so we build the string by iterating the vector in reverse.
        for (int i = parts.size() - 1; i >= 0; --i) {
            result += parts[i];
            if (i > 0) {
                result += " + ";
            }
        }
        return result;
    }
};

int main() {
    long long testCases[] = {12, 42, 70304, 9000000, 806};
    for (long long testCase : testCases) {
        std::cout << "Input: " << testCase << ", Output: " << Task60::expandedForm(testCase) << std::endl;
    }
    return 0;
}