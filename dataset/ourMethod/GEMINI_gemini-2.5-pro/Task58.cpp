#include <iostream>
#include <string>
#include <cctype>
#include <vector>
#include <algorithm>

// Checks to see if a string has the same amount of 'x's and 'o's.
// The method is case insensitive. The string can contain any char.
bool XO(const std::string& str) {
    int x_count = 0;
    int o_count = 0;

    for (char c : str) {
        // Use std::tolower for case-insensitivity.
        // Cast to unsigned char is the standard safe way to use ctype functions.
        char lower_c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        if (lower_c == 'x') {
            x_count++;
        } else if (lower_c == 'o') {
            o_count++;
        }
    }

    return x_count == o_count;
}

int main() {
    std::vector<std::string> test_cases = {
        "ooxx",
        "xooxx",
        "ooxXm",
        "zpzpzpp",
        "zzoo"
    };

    std::cout << std::boolalpha;
    for (const auto& test_str : test_cases) {
        bool result = XO(test_str);
        std::cout << "XO(\"" << test_str << "\") => " << result << std::endl;
    }

    return 0;
}