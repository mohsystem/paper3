#include <iostream>
#include <string>
#include <vector>

std::string expandedForm(int num) {
    std::string s = std::to_string(num);
    std::string result = "";
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] != '0') {
            if (!result.empty()) {
                result += " + ";
            }
            result += s[i];
            for (int j = 0; j < s.length() - 1 - i; ++j) {
                result += '0';
            }
        }
    }
    return result;
}

int main() {
    std::cout << "12 -> " << expandedForm(12) << std::endl;
    std::cout << "42 -> " << expandedForm(42) << std::endl;
    std::cout << "70304 -> " << expandedForm(70304) << std::endl;
    std::cout << "999 -> " << expandedForm(999) << std::endl;
    std::cout << "800000 -> " << expandedForm(800000) << std::endl;
    return 0;
}