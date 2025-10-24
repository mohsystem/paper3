#include <iostream>
#include <string>

std::string encrypt(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') {
            out.push_back(static_cast<char>('A' + (c - 'A' + 3) % 26));
        } else if (c >= 'a' && c <= 'z') {
            out.push_back(static_cast<char>('a' + (c - 'a' + 3) % 26));
        } else if (c >= '0' && c <= '9') {
            out.push_back(static_cast<char>('0' + (c - '0' + 5) % 10));
        } else {
            out.push_back(c);
        }
    }
    return out;
}

int main() {
    std::string tests[5] = {
        "Hello, World!",
        "xyz XYZ",
        "Encrypt123",
        "",
        "Attack at dawn! 09"
    };
    for (const auto& t : tests) {
        std::cout << "Input:  " << t << "\n";
        std::cout << "Output: " << encrypt(t) << "\n\n";
    }
    return 0;
}