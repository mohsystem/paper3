/*
Chain-of-Through process:
1) Problem understanding: Read user input from console and try converting to 32-bit int; provide function that takes a parameter and returns a result.
2) Security requirements: Validate length, characters; handle range and conversion errors safely; avoid exceptions.
3) Secure coding generation: Use strtol with errno for safe conversion; trim and validate input.
4) Code review: Checked for proper bounds, full consumption of input, and safe I/O usage.
5) Secure code output: Final code includes safe parsing, console read, and 5 test cases.
*/

#include <iostream>
#include <string>
#include <climits>
#include <cerrno>
#include <cstdlib>
#include <cctype>
#include <vector>

struct ParseResult {
    bool success;
    int value;
    std::string error;
};

static std::string trim(const std::string& in) {
    size_t start = 0;
    while (start < in.size() && std::isspace(static_cast<unsigned char>(in[start]))) start++;
    if (start == in.size()) return std::string();
    size_t end = in.size();
    while (end > start && std::isspace(static_cast<unsigned char>(in[end - 1]))) end--;
    return in.substr(start, end - start);
}

// Function that accepts input as parameter and returns output
ParseResult tryParseInt(const std::string& input) {
    std::string s = trim(input);

    if (s.empty()) {
        return {false, 0, "Empty input"};
    }
    if (s.size() > 1000) {
        return {false, 0, "Input too long"};
    }

    // Validate characters: optional sign followed by digits
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (i == 0 && (c == '+' || c == '-')) continue;
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            return {false, 0, "Invalid characters"};
        }
    }

    errno = 0;
    char* endptr = nullptr;
    long val = std::strtol(s.c_str(), &endptr, 10);

    if (errno == ERANGE || val < INT_MIN || val > INT_MAX) {
        return {false, 0, "Number out of int range"};
    }
    if (endptr == nullptr || *endptr != '\0') {
        return {false, 0, "Invalid characters"};
    }

    return {true, static_cast<int>(val), ""};
}

int main() {
    std::cout << "Enter a value to convert to integer:" << std::endl;
    std::string line;
    if (std::getline(std::cin, line)) {
        ParseResult res = tryParseInt(line);
        if (res.success) {
            std::cout << "Success: " << res.value << std::endl;
        } else {
            std::cout << "Error: " << res.error << std::endl;
        }
    } else {
        std::cout << "No input received." << std::endl;
    }

    // 5 test cases
    std::vector<std::string> tests = {"123", "  -42  ", "2147483647", "2147483648", "abc"};
    std::cout << "---- Test Cases ----" << std::endl;
    for (const auto& t : tests) {
        ParseResult r = tryParseInt(t);
        std::cout << "Input: \"" << t << "\" -> " << (r.success ? "Success: " + std::to_string(r.value) : "Error: " + r.error) << std::endl;
    }

    return 0;
}