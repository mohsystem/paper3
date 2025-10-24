#include <iostream>
#include <string>
#include <array>

std::string longest(const std::string& s1, const std::string& s2) {
    std::array<bool, 26> seen{};
    auto process = [&seen](const std::string& s) {
        for (char c : s) {
            if (c >= 'a' && c <= 'z') {
                seen[static_cast<size_t>(c - 'a')] = true;
            }
        }
    };
    process(s1);
    process(s2);
    std::string res;
    res.reserve(26);
    for (int i = 0; i < 26; ++i) {
        if (seen[static_cast<size_t>(i)]) {
            res.push_back(static_cast<char>('a' + i));
        }
    }
    return res;
}

int main() {
    const std::string A[5] = {
        "xyaabbbccccdefww",
        "abcdefghijklmnopqrstuvwxyz",
        "",
        "abc123",
        "thequickbrownfox"
    };
    const std::string B[5] = {
        "xxxxyyyyabklmopq",
        "abcdefghijklmnopqrstuvwxyz",
        "",
        "def!ghi",
        "jumpsoverthelazydog"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << longest(A[i], B[i]) << '\n';
    }
    return 0;
}