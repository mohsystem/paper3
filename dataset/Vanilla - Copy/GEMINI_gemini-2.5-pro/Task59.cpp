#include <iostream>
#include <string>
#include <cctype>

std::string accum(const std::string& s) {
    if (s.empty()) {
        return "";
    }
    std::string result = "";
    for (size_t i = 0; i < s.length(); ++i) {
        result += std::toupper(static_cast<unsigned char>(s[i]));
        for (size_t j = 0; j < i; ++j) {
            result += std::tolower(static_cast<unsigned char>(s[i]));
        }
        if (i < s.length() - 1) {
            result += "-";
        }
    }
    return result;
}

int main() {
    std::cout << "Input: \"abcd\" -> Output: \"" << accum("abcd") << "\"" << std::endl;
    std::cout << "Input: \"RqaEzty\" -> Output: \"" << accum("RqaEzty") << "\"" << std::endl;
    std::cout << "Input: \"cwAt\" -> Output: \"" << accum("cwAt") << "\"" << std::endl;
    std::cout << "Input: \"Z\" -> Output: \"" << accum("Z") << "\"" << std::endl;
    std::cout << "Input: \"Mumbling\" -> Output: \"" << accum("Mumbling") << "\"" << std::endl;
    return 0;
}