#include <iostream>
#include <string>
#include <cctype>

std::string process(const std::string& s) {
    if (s.empty()) return "";
    size_t i = 0, j = s.size();
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    if (i >= j) return "";
    std::string out;
    out.reserve(j - i);
    bool prevSpace = false;
    for (size_t k = i; k < j; ++k) {
        unsigned char ch = static_cast<unsigned char>(s[k]);
        if (std::isspace(ch)) {
            if (!prevSpace) {
                out.push_back(' ');
                prevSpace = true;
            }
        } else {
            out.push_back(static_cast<char>(std::toupper(ch)));
            prevSpace = false;
        }
    }
    return out;
}

int main() {
    std::string userInput;
    if (std::getline(std::cin, userInput)) {
        std::cout << process(userInput) << std::endl;
    }

    const char* tests[5] = {
        "  hello   world  ",
        "\tMixed\tCase Input\n",
        "already CLEAN",
        "   ",
        "Symbols 123 !@#   test"
    };
    for (const char* t : tests) {
        std::cout << process(std::string(t)) << std::endl;
    }
    return 0;
}