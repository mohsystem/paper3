
#include <iostream>
#include <string>
#include <cctype>

std::string accum(const std::string& s) {
    if (s.empty()) {
        return "";
    }
    
    std::string result;
    for (size_t i = 0; i < s.length(); i++) {
        result += toupper(s[i]);
        for (size_t j = 0; j < i; j++) {
            result += tolower(s[i]);
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
