#include <iostream>
#include <string>
#include <vector>

static const size_t MAX_LEN = 64;

bool parseToInt(const std::string& input, int& out) {
    // Trim spaces
    size_t start = input.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return false;
    size_t end = input.find_last_not_of(" \t\r\n");
    std::string s = input.substr(start, end - start + 1);

    if (s.empty() || s.size() > MAX_LEN) return false;

    size_t i = 0;
    bool negative = false;
    if (s[0] == '-') { negative = true; i = 1; }
    else if (s[0] == '+') { i = 1; }

    if (i == s.size()) return false;

    int limit = negative ? INT32_MIN : -INT32_MAX;
    int multmin = limit / 10;
    int result = 0;

    for (; i < s.size(); ++i) {
        char ch = s[i];
        if (ch < '0' || ch > '9') return false;
        int digit = ch - '0';
        if (result < multmin) return false;
        result *= 10;
        if (result < limit + digit) return false;
        result -= digit;
    }

    out = negative ? result : -result;
    return true;
}

int main(int argc, char* argv[]) {
    std::vector<std::string> inputs;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) inputs.emplace_back(argv[i]);
    } else {
        inputs = {"123", "-42", "00123", "2147483648", "abc123"};
    }

    for (const auto& s : inputs) {
        int value = 0;
        if (parseToInt(s, value)) {
            std::cout << "Input: \"" << s << "\" -> OK: " << value << "\n";
        } else {
            std::cout << "Input: \"" << s << "\" -> Invalid integer\n";
        }
    }
    return 0;
}