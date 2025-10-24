#include <iostream>
#include <string>

std::string spinWords(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    size_t i = 0, n = s.size();
    while (i < n) {
        if (s[i] == ' ') {
            result.push_back(' ');
            ++i;
        } else {
            size_t start = i;
            while (i < n && s[i] != ' ') ++i;
            size_t len = i - start;
            if (len >= 5) {
                for (size_t j = 0; j < len; ++j) result.push_back(s[start + len - 1 - j]);
            } else {
                result.append(s, start, len);
            }
        }
    }
    return result;
}

int main() {
    std::string tests[5] = {
        "Hey fellow warriors",
        "This is a test",
        "This is another test",
        "Welcome",
        "CodeWars is nice place"
    };
    for (const auto& t : tests) {
        std::cout << spinWords(t) << std::endl;
    }
    return 0;
}