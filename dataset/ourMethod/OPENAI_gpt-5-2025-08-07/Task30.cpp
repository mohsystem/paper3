#include <iostream>
#include <string>
#include <array>

static bool is_valid_lower_alpha(const std::string& s) {
    for (char ch : s) {
        if (ch < 'a' || ch > 'z') {
            return false;
        }
    }
    return true;
}

std::string longest(const std::string& s1, const std::string& s2) {
    if (!is_valid_lower_alpha(s1) || !is_valid_lower_alpha(s2)) {
        std::cerr << "Error: inputs must contain only 'a' to 'z'.\n";
        return std::string();
    }

    std::array<bool, 26> seen{};
    for (char ch : s1) {
        seen[static_cast<size_t>(ch - 'a')] = true;
    }
    for (char ch : s2) {
        seen[static_cast<size_t>(ch - 'a')] = true;
    }

    std::string result;
    result.reserve(26);
    for (size_t i = 0; i < 26; ++i) {
        if (seen[i]) {
            result.push_back(static_cast<char>('a' + i));
        }
    }
    return result;
}

int main() {
    // 5 test cases
    struct TestCase {
        std::string a;
        std::string b;
    } tests[] = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"", ""},
        {"aaaa", "bbb"},
        {"abcxyz", "mnop"}
    };

    for (const auto& t : tests) {
        std::string out = longest(t.a, t.b);
        std::cout << "longest(\"" << t.a << "\", \"" << t.b << "\") -> \"" << out << "\"\n";
    }

    return 0;
}