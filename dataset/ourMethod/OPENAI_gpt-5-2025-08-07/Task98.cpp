#include <iostream>
#include <string>
#include <limits>
#include <cctype>

struct ParseResult {
    bool success;
    int value;
    std::string error;
};

static const std::size_t MAX_LEN = 64;

ParseResult parseIntStrict(const std::string& input) {
    ParseResult res{false, 0, ""};
    // Trim
    std::size_t start = 0;
    std::size_t end = input.size();
    while (start < end && std::isspace(static_cast<unsigned char>(input[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(input[end - 1]))) end--;

    if (start == end) {
        res.error = "Empty input";
        return res;
    }
    if ((end - start) > MAX_LEN) {
        res.error = "Input too long";
        return res;
    }

    bool negative = false;
    std::size_t idx = start;
    if (input[idx] == '+' || input[idx] == '-') {
        negative = (input[idx] == '-');
        idx++;
        if (idx >= end) {
            res.error = "No digits after sign";
            return res;
        }
    }

    long long acc = 0;
    for (std::size_t i = idx; i < end; ++i) {
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < '0' || c > '9') {
            res.error = "Invalid character encountered";
            return res;
        }
        int d = c - '0';
        acc = acc * 10 + d;
        long long limit = negative ? (-(long long)std::numeric_limits<int>::min()) : (long long)std::numeric_limits<int>::max();
        if (acc > limit) {
            res.error = "Integer out of range";
            return res;
        }
    }

    long long signedVal = negative ? -acc : acc;
    if (signedVal < std::numeric_limits<int>::min() || signedVal > std::numeric_limits<int>::max()) {
        res.error = "Integer out of range";
        return res;
    }

    res.success = true;
    res.value = static_cast<int>(signedVal);
    return res;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Enter value #" << i << ": ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "No more input." << std::endl;
            break;
        }
        ParseResult r = parseIntStrict(line);
        if (r.success) {
            std::cout << "Parsed integer: " << r.value << std::endl;
        } else {
            std::cout << "Error: " << r.error << std::endl;
        }
    }
    return 0;
}