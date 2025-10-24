#include <iostream>
#include <string>
#include <cctype>

class Task59 {
public:
    static std::string accum(const std::string& s) {
        std::string result = "";
        for (size_t i = 0; i < s.length(); ++i) {
            // Append the first character, capitalized
            result += toupper(static_cast<unsigned char>(s[i]));
            // Append the rest of the characters, in lowercase, i times
            for (size_t j = 0; j < i; ++j) {
                result += tolower(static_cast<unsigned char>(s[i]));
            }
            // Append a hyphen if it's not the last part
            if (i < s.length() - 1) {
                result += "-";
            }
        }
        return result;
    }
};

int main() {
    std::string test_cases[] = {"abcd", "RqaEzty", "cwAt", "Z", ""};
    for (const auto& test_case : test_cases) {
        std::cout << "\"" << test_case << "\" -> \"" << Task59::accum(test_case) << "\"" << std::endl;
    }
    return 0;
}