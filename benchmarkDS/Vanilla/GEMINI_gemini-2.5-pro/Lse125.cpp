#include <iostream>
#include <string>
#include <vector>

std::string trimTrailing(const std::string& str) {
    const std::string whitespace = " \t\n\r\f\v";
    size_t end = str.find_last_not_of(whitespace);
    if (std::string::npos == end) {
        return "";
    }
    return str.substr(0, end + 1);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string input(argv[1]);
        std::cout << trimTrailing(input) << std::endl;
    } else {
        std::cout << "--- Running Test Cases ---" << std::endl;
        std::vector<std::string> testCases = {
            "Hello World   ",
            "  leading and trailing  ",
            "no_whitespace",
            "   \t\n",
            "Trailing tab\t"
        };
        for (const auto& test : testCases) {
            std::cout << "Original: [" << test << "]" << std::endl;
            std::cout << "Trimmed:  [" << trimTrailing(test) << "]" << std::endl;
            std::cout << std::endl;
        }
    }
    return 0;
}