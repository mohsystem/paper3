#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <climits>

struct Result {
    bool success;
    int value;
    std::string error;
};

static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) start++;
    if (start == str.size()) return "";
    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) end--;
    return str.substr(start, end - start + 1);
}

Result convertToInt(const std::string& input) {
    std::string s = trim(input);
    if (s.empty()) return {false, 0, "empty input"};

    int sign = 1;
    size_t idx = 0;
    if (s[0] == '+' || s[0] == '-') {
        sign = (s[0] == '-') ? -1 : 1;
        idx++;
    }
    if (idx >= s.size()) return {false, 0, "no digits"};

    long long acc = 0;
    long long limit = (sign == 1) ? INT_MAX : -(long long)INT_MIN; // 2147483648 for negative

    for (size_t i = idx; i < s.size(); ++i) {
        char c = s[i];
        if (c < '0' || c > '9') {
            return {false, 0, std::string("invalid character at position ") + std::to_string(i)};
        }
        int digit = c - '0';
        if (acc > (limit - digit) / 10LL) {
            return {false, 0, "overflow"};
        }
        acc = acc * 10LL + digit;
    }
    int value = (sign == 1) ? static_cast<int>(acc) : static_cast<int>(-acc);
    return {true, value, ""};
}

int main(int argc, char* argv[]) {
    std::vector<std::string> inputs;
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) inputs.emplace_back(argv[i]);
    } else {
        inputs = {"123", "-42", "2147483647", "-2147483648", "99abc"};
    }

    for (const auto& s : inputs) {
        Result r = convertToInt(s);
        if (r.success) {
            std::cout << "Input: \"" << s << "\" -> OK " << r.value << "\n";
        } else {
            std::cout << "Input: \"" << s << "\" -> ERROR " << r.error << "\n";
        }
    }
    return 0;
}