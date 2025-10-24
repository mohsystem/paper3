
#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>

class Task60 {
public:
    static std::string expandedForm(int num) {
        if (num <= 0) {
            throw std::invalid_argument("Number must be greater than 0");
        }
        
        std::string numStr = std::to_string(num);
        std::ostringstream result;
        int length = numStr.length();
        bool first = true;
        
        for (int i = 0; i < length; i++) {
            if (numStr[i] != '0') {
                if (!first) {
                    result << " + ";
                }
                result << numStr[i];
                for (int j = i + 1; j < length; j++) {
                    result << '0';
                }
                first = false;
            }
        }
        
        return result.str();
    }
};

int main() {
    // Test cases
    std::cout << Task60::expandedForm(12) << std::endl;      // "10 + 2"
    std::cout << Task60::expandedForm(42) << std::endl;      // "40 + 2"
    std::cout << Task60::expandedForm(70304) << std::endl;   // "70000 + 300 + 4"
    std::cout << Task60::expandedForm(9) << std::endl;       // "9"
    std::cout << Task60::expandedForm(100000) << std::endl;  // "100000"
    
    return 0;
}
