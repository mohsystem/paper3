#include <iostream>
#include <string>
#include <vector>
#include <limits>
#include <cctype>

static bool string_to_int(const std::string& s, int& out) {
    // Parse non-negative decimal with overflow and format checks
    std::string t;
    t.reserve(s.size());
    // trim
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    t = s.substr(start, end - start);
    if (t.empty()) return false;

    size_t i = 0;
    if (t[0] == '-') return false; // negative not allowed
    if (t[0] == '+') {
        i = 1;
        if (i >= t.size()) return false;
    }

    int result = 0;
    const int INT_MAX_V = std::numeric_limits<int>::max();
    for (; i < t.size(); ++i) {
        char ch = t[i];
        if (ch < '0' || ch > '9') return false;
        int digit = ch - '0';
        if (result > (INT_MAX_V - digit) / 10) return false; // overflow
        result = result * 10 + digit;
    }
    out = result;
    return true;
}

static bool print_arg_at_index(const std::vector<std::string>& argv, const std::string& indexStr) {
    if (argv.empty()) {
        std::cerr << "ERROR" << std::endl;
        return false;
    }
    int idx = 0;
    if (!string_to_int(indexStr, idx)) {
        std::cerr << "ERROR" << std::endl;
        return false;
    }
    if (idx < 0 || static_cast<size_t>(idx) >= argv.size()) {
        std::cerr << "ERROR" << std::endl;
        return false;
    }
    std::cout << argv[static_cast<size_t>(idx)] << std::endl;
    return true;
}

static void run_tests() {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {"x", "2", "a", "b", "c", "d"},       // prints "a"
        {"index", "0", "hello"},              // prints "index"
        {"index", "3", "p", "q", "r", "s"},   // prints "s"
        {"cmd", "100", "a", "b"},             // ERROR (stderr)
        {"cmd", "abc", "z"}                   // ERROR (stderr)
    };
    for (const auto& t : tests) {
        if (t.size() > 1) {
            print_arg_at_index(t, t[1]);
        } else {
            std::cerr << "ERROR" << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    // Run 5 test cases
    run_tests();

    // Also process real command line input (excluding program name)
    if (argc > 2) {
        std::vector<std::string> args_only;
        args_only.reserve(static_cast<size_t>(argc - 1));
        for (int i = 1; i < argc; ++i) {
            args_only.emplace_back(argv[i]);
        }
        // index string is the second argument => original argv[2], now args_only[1]
        print_arg_at_index(args_only, args_only[1]);
    }
    return 0;
}