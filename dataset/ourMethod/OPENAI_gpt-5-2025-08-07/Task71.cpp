#include <iostream>
#include <string>
#include <charconv>
#include <cctype>
#include <vector>
#include <limits>
#include <system_error>

struct ParseResult {
    bool ok;
    long long value;
    std::string error;
};

static std::string trim_copy(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

static ParseResult parse_integer(const std::string& input, std::size_t maxLen = 128) {
    ParseResult r{false, 0, ""};
    std::string s = trim_copy(input);
    if (s.empty()) {
        r.error = "empty input";
        return r;
    }
    if (s.size() > maxLen) {
        r.error = "input too long";
        return r;
    }

    long long value = 0;
    const char* first = s.data();
    const char* last = s.data() + s.size();
    auto res = std::from_chars(first, last, value, 10);

    if (res.ec == std::errc::invalid_argument) {
        r.error = "invalid integer format";
        return r;
    }
    if (res.ec == std::errc::result_out_of_range) {
        r.error = "integer overflow";
        return r;
    }
    if (res.ptr != last) {
        r.error = "invalid trailing characters";
        return r;
    }

    r.ok = true;
    r.value = value;
    return r;
}

static void run_and_print(const std::string& input) {
    ParseResult res = parse_integer(input);
    std::cout << "Input: \"" << input << "\" -> ";
    if (res.ok) {
        std::cout << "OK, value=" << res.value << "\n";
    } else {
        std::cout << "ERROR, " << res.error << "\n";
    }
}

int main(int argc, char* argv[]) {
    // If user provided command-line arguments, process them; otherwise, run 5 test cases.
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            run_and_print(std::string(argv[i] ? argv[i] : ""));
        }
    } else {
        // 5 test cases
        std::vector<std::string> tests{
            "123",
            "-42",
            "  17",
            "9223372036854775808", // overflow for 64-bit
            "12abc"
        };
        for (const auto& t : tests) {
            run_and_print(t);
        }
    }
    return 0;
}