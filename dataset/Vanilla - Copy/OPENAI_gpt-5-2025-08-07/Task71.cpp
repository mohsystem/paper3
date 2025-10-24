#include <iostream>
#include <string>
#include <optional>
#include <charconv>
#include <cctype>

std::string trim(const std::string& s) {
    const std::string ws = " \t\n\r\f\v";
    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

// Converts string to int; returns empty optional if invalid
std::optional<int> convertToInt(const std::string& s) {
    std::string t = trim(s);
    if (t.empty()) return std::nullopt;

    int value = 0;
    const char* first = t.data();
    const char* last = t.data() + t.size();
    std::from_chars_result res = std::from_chars(first, last, value, 10);
    if (res.ec != std::errc() || res.ptr != last) {
        return std::nullopt;
    }
    return value;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string a = argv[i];
            auto r = convertToInt(a);
            if (r.has_value()) {
                std::cout << "Input: \"" << a << "\" -> " << r.value() << "\n";
            } else {
                std::cout << "Input: \"" << a << "\" -> invalid\n";
            }
        }
    } else {
        std::string tests[5] = {"123", "  42 ", "-7", "abc", "2147483648"};
        for (const auto& t : tests) {
            auto r = convertToInt(t);
            if (r.has_value()) {
                std::cout << "Input: \"" << t << "\" -> " << r.value() << "\n";
            } else {
                std::cout << "Input: \"" << t << "\" -> invalid\n";
            }
        }
    }
    return 0;
}