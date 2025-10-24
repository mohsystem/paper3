#include <iostream>
#include <string>
#include <cctype>
#include <vector>

// Check if the string has the same amount of 'x' and 'o' (case-insensitive).
// Returns true if counts are equal (including when both are zero), false otherwise.
bool XO(const std::string& s) {
    // Basic input validation (length check to avoid excessive processing)
    const size_t MAX_LEN = 10'000'000u;
    if (s.size() > MAX_LEN) {
        return false; // fail closed on unreasonable input size
    }

    size_t countX = 0;
    size_t countO = 0;

    for (unsigned char ch : s) {
        unsigned char lower = static_cast<unsigned char>(std::tolower(ch));
        if (lower == static_cast<unsigned char>('x')) {
            ++countX;
        } else if (lower == static_cast<unsigned char>('o')) {
            ++countO;
        }
    }
    return countX == countO;
}

int main() {
    std::vector<std::string> tests = {
        "ooxx",
        "xooxx",
        "ooxXm",
        "zpzpzpp",
        "zzoo"
    };

    std::cout << std::boolalpha;
    for (const auto& t : tests) {
        bool result = XO(t);
        std::cout << "XO(\"" << t << "\") => " << result << "\n";
    }

    return 0;
}