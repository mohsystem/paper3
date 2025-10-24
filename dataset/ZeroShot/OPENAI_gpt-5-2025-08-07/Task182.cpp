#include <iostream>
#include <string>
#include <vector>
#include <cctype>

class Task182 {
public:
    static std::string cleanNumber(const std::string& s) {
        if (s.empty()) return std::string();
        std::string digits;
        digits.reserve(16);
        for (unsigned char ch : s) {
            if (std::isdigit(ch)) {
                digits.push_back(static_cast<char>(ch));
            }
        }
        if (digits.size() == 11) {
            if (digits[0] != '1') return std::string();
            digits.erase(digits.begin());
        }
        if (digits.size() != 10) return std::string();
        if (digits[0] < '2' || digits[0] > '9') return std::string();
        if (digits[3] < '2' || digits[3] > '9') return std::string();
        return digits;
    }
};

int main() {
    std::vector<std::string> tests = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "123-456-7890" // invalid
    };
    for (const auto& t : tests) {
        std::cout << Task182::cleanNumber(t) << "\n";
    }
    return 0;
}