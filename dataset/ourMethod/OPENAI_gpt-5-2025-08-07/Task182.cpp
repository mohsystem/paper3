#include <iostream>
#include <string>
#include <cctype>

static std::string clean_number(const std::string& s) {
    std::string digits;
    digits.reserve(s.size());
    for (unsigned char uc : s) {
        if (std::isdigit(uc)) {
            digits.push_back(static_cast<char>(uc));
        }
    }

    if (digits.size() == 11 && digits[0] == '1') {
        digits.erase(digits.begin());
    } else if (digits.size() != 10) {
        return std::string();
    }

    if (digits[0] < '2' || digits[0] > '9') return std::string();
    if (digits[3] < '2' || digits[3] > '9') return std::string();

    return digits;
}

int main() {
    const std::string tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "1 (123) 456-7890"
    };
    for (const auto& t : tests) {
        std::string cleaned = clean_number(t);
        std::cout << cleaned << "\n";
    }
    return 0;
}