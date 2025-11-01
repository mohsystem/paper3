#include <iostream>
#include <string>
#include <cctype>
#include <climits>
#include <cerrno>
#include <cstdlib>

struct AddResult {
    bool success;
    long long value;
};

static bool is_valid_integer_string(const std::string& s, std::size_t max_len = 100) {
    if (s.empty() || s.size() > max_len) return false;
    std::size_t start = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s.size() == 1) return false;
        start = 1;
    }
    for (std::size_t i = start; i < s.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(s[i]))) return false;
    }
    return true;
}

static bool parse_long_long(const std::string& s, long long& out) {
    if (!is_valid_integer_string(s)) return false;
    errno = 0;
    char* endptr = nullptr;
    const char* cstr = s.c_str();
    long long val = std::strtoll(cstr, &endptr, 10);
    if (errno != 0 || endptr == cstr || *endptr != '\0') {
        return false;
    }
    out = val;
    return true;
}

AddResult add_thousand(long long input) {
    AddResult res;
    if (input > LLONG_MAX - 1000LL || input < LLONG_MIN + 1000LL) {
        res.success = false;
        res.value = 0;
        return res;
    }
    res.success = true;
    res.value = input + 1000LL;
    return res;
}

int main(int argc, char* argv[]) {
    // 5 test cases
    const char* tests[] = {
        "-1000",
        "0",
        "42",
        "2147483647",
        "9223372036854774807"
    };
    for (const char* t : tests) {
        std::string s(t);
        long long v = 0;
        if (!parse_long_long(s, v)) {
            std::cout << "Invalid test input: " << s << "\n";
        } else {
            AddResult r = add_thousand(v);
            if (r.success) {
                std::cout << "add_thousand(" << s << ") = " << r.value << "\n";
            } else {
                std::cout << "Overflow in test input: " << s << "\n";
            }
        }
    }

    // Command-line processing
    if (argc >= 2) {
        std::string arg = argv[1];
        long long v = 0;
        if (!parse_long_long(arg, v)) {
            std::cerr << "Invalid input\n";
            return 1;
        }
        AddResult r = add_thousand(v);
        if (!r.success) {
            std::cerr << "Invalid input\n";
            return 1;
        }
        std::cout << r.value << "\n";
    }
    return 0;
}