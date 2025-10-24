#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

class Task60 {
public:
    /**
     * Converts a positive number into its expanded form string.
     *
     * @param num The number to convert, must be greater than 0.
     * @return The expanded form of the number as a string.
     */
    static std::string expandedForm(long long num) {
        // According to the problem description, num will be > 0.
        // Adding a check for robustness.
        if (num <= 0) {
            return "";
        }
        
        std::string s = std::to_string(num);
        std::vector<std::string> parts;
        int len = s.length();

        for (int i = 0; i < len; ++i) {
            if (s[i] != '0') {
                std::string part;
                part += s[i];
                part.append(len - 1 - i, '0');
                parts.push_back(part);
            }
        }
        
        if (parts.empty()) {
            return "";
        }

        std::string result = parts[0];
        for (size_t i = 1; i < parts.size(); ++i) {
            result += " + " + parts[i];
        }
        return result;
    }
};

int main() {
    // Test Cases
    std::cout << "12 -> " << Task60::expandedForm(12) << std::endl; // Expected: 10 + 2
    std::cout << "42 -> " << Task60::expandedForm(42) << std::endl; // Expected: 40 + 2
    std::cout << "70304 -> " << Task60::expandedForm(70304) << std::endl; // Expected: 70000 + 300 + 4
    std::cout << "9000000 -> " << Task60::expandedForm(9000000) << std::endl; // Expected: 9000000
    std::cout << "1 -> " << Task60::expandedForm(1) << std::endl; // Expected: 1
    return 0;
}