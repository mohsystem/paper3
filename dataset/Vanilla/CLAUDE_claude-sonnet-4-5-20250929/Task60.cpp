
#include <iostream>
#include <string>
#include <vector>

std::string expandedForm(int num) {
    std::string numStr = std::to_string(num);
    std::string result = "";
    int length = numStr.length();
    
    for (int i = 0; i < length; i++) {
        char digit = numStr[i];
        if (digit != '0') {
            if (!result.empty()) {
                result += " + ";
            }
            result += digit;
            for (int j = 0; j < length - i - 1; j++) {
                result += '0';
            }
        }
    }
    
    return result;
}

int main() {
    std::cout << expandedForm(12) << std::endl;      // "10 + 2"
    std::cout << expandedForm(42) << std::endl;      // "40 + 2"
    std::cout << expandedForm(70304) << std::endl;   // "70000 + 300 + 4"
    std::cout << expandedForm(9000) << std::endl;    // "9000"
    std::cout << expandedForm(123456) << std::endl;  // "100000 + 20000 + 3000 + 400 + 50 + 6"
    return 0;
}
