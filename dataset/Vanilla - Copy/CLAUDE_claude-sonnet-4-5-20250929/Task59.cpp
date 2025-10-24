
#include <iostream>
#include <string>
#include <cctype>

std::string accum(const std::string& s) {
    std::string result = "";
    for (size_t i = 0; i < s.length(); i++) {
        char c = s[i];
        result += std::toupper(c);
        for (size_t j = 0; j < i; j++) {
            result += std::tolower(c);
        }
        if (i < s.length() - 1) {
            result += "-";
        }
    }
    return result;
}

int main() {
    std::cout << accum("abcd") << std::endl;
    std::cout << accum("RqaEzty") << std::endl;
    std::cout << accum("cwAt") << std::endl;
    std::cout << accum("ZpglnRxqenU") << std::endl;
    std::cout << accum("NyffsGeyylB") << std::endl;
    return 0;
}
