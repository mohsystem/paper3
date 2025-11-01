#include <iostream>
#include <string>
#include <climits>
#include <cctype>
#include <charconv>

// Safely adds 1000 to a long long. Returns result via 'out' and true on success, false on overflow.
bool add1000(long long in, long long& out) {
    if (in > LLONG_MAX - 1000) {
        return false; // would overflow
    }
    out = in + 1000;
    return true;
}

// Trim leading/trailing whitespace
static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

// Parse signed long long from string (base 10) with validation
bool parse_ll(const std::string& input, long long& out) {
    std::string s = trim(input);
    if (s.empty()) return false;
    const char* first = s.data();
    const char* last = s.data() + s.size();
    std::from_chars_result res = std::from_chars(first, last, out, 10);
    if (res.ec != std::errc() || res.ptr != last) {
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Process command line input: read a value, add 1000, print result
    if (argc > 1 && argv[1] != nullptr) {
        long long v = 0, r = 0;
        if (parse_ll(argv[1], v)) {
            if (add1000(v, r)) {
                std::cout << r << std::endl;
            } else {
                std::cerr << "Overflow when adding 1000" << std::endl;
            }
        } else {
            std::cerr << "Invalid integer input" << std::endl;
        }
    }

    // 5 test cases
    long long tests[5];
    tests[0] = 0;
    tests[1] = 12345;
    tests[2] = -1000;
    tests[3] = LLONG_MAX - 999; // safe
    tests[4] = LLONG_MAX - 998; // will overflow when adding 1000

    for (int i = 0; i < 5; ++i) {
        long long out = 0;
        if (add1000(tests[i], out)) {
            std::cout << out << std::endl;
        } else {
            std::cerr << "Test overflow for input: " << tests[i] << std::endl;
        }
    }

    return 0;
}