#include <iostream>
#include <string>
#include <vector>

std::string spinWords(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    size_t n = s.size();
    size_t i = 0;
    while (i < n) {
        if (s[i] == ' ') {
            result.push_back(' ');
            ++i;
        } else {
            size_t start = i;
            while (i < n && s[i] != ' ') {
                ++i;
            }
            size_t len = i - start;
            if (len >= 5) {
                for (size_t k = 0; k < len; ++k) {
                    result.push_back(s[i - 1 - k]);
                }
            } else {
                result.append(s, start, len);
            }
        }
    }
    return result;
}

int main() {
    std::vector<std::string> tests = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "",
        "Hi   there"
    };
    for (const auto& t : tests) {
        std::cout << "Input: \"" << t << "\" -> Output: \"" << spinWords(t) << "\"\n";
    }
    return 0;
}