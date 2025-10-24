// Task182 - C++ implementation
// Chain-of-Through process:
// 1) Understand: Clean NANP numbers; remove punctuation, handle optional country code '1', ensure NXX NXX-XXXX.
// 2) Security: No unsafe casts, handle large inputs, no buffer overflow.
// 3) Secure coding: Use std::string and validations.
// 4) Review: Validate lengths and leading digits.
// 5) Output: Return cleaned string or empty string if invalid.

#include <iostream>
#include <string>
#include <cctype>

std::string clean(const std::string& input) {
    std::string digits;
    digits.reserve(input.size());
    for (char c : input) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            digits.push_back(c);
        }
    }
    if (digits.size() == 11) {
        if (digits[0] != '1') return "";
        digits.erase(digits.begin());
    }
    if (digits.size() != 10) return "";
    if (digits[0] < '2' || digits[0] > '9') return "";
    if (digits[3] < '2' || digits[3] > '9') return "";
    return digits;
}

int main() {
    const std::string tests[5] = {
        "+1 (613)-995-0253",
        "613-995-0253",
        "1 613 995 0253",
        "613.995.0253",
        "1 123 456 7890"
    };
    for (const auto& t : tests) {
        std::cout << clean(t) << std::endl;
    }
    return 0;
}