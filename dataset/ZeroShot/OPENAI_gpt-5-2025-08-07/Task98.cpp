#include <iostream>
#include <string>
#include <charconv>
#include <algorithm>
#include <cctype>
#include <limits>

struct ParseResult {
    bool ok;
    long long value;
};

static std::string trim(const std::string& s) {
    auto is_not_space = [](unsigned char ch) { return !std::isspace(ch); };
    auto start = std::find_if(s.begin(), s.end(), is_not_space);
    if (start == s.end()) return std::string();
    auto rstart = std::find_if(s.rbegin(), s.rend(), is_not_space).base();
    return std::string(start, rstart);
}

ParseResult try_parse_long_long(const std::string& input) {
    std::string t = trim(input);
    if (t.empty()) return {false, 0};

    long long value = 0;
    const char* first = t.c_str();
    const char* last = first + t.size();
    std::from_chars_result res = std::from_chars(first, last, value, 10);
    if (res.ec != std::errc() || res.ptr != last) {
        return {false, 0};
    }
    return {true, value};
}

int main() {
    std::string line;
    if (std::getline(std::cin, line)) {
        ParseResult r = try_parse_long_long(line);
        if (r.ok) {
            std::cout << "Parsed integer: " << r.value << "\n";
        } else {
            std::cout << "Invalid integer input.\n";
        }
    } else {
        std::cout << "No input provided.\n";
    }

    const std::string tests[5] = {
        "123",
        "   -456   ",
        "+789",
        "9223372036854775808",
        "abc123"
    };
    for (const auto& t : tests) {
        ParseResult r = try_parse_long_long(t);
        if (r.ok) {
            std::cout << "Test \"" << t << "\" -> " << r.value << "\n";
        } else {
            std::cout << "Test \"" << t << "\" -> invalid\n";
        }
    }
    return 0;
}