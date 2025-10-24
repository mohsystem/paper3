#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <cctype>

static const std::size_t MAX_LEN = 100000;

static void validateString(const std::string& s) {
    if (s.size() > MAX_LEN) {
        throw std::invalid_argument("Input string exceeds maximum allowed length.");
    }
}

std::string copyString(const std::string& s) {
    validateString(s);
    return std::string(s);
}

std::string concatStrings(const std::string& a, const std::string& b) {
    validateString(a);
    validateString(b);
    if (a.size() > std::numeric_limits<std::size_t>::max() - b.size()) {
        throw std::invalid_argument("Size overflow in concatenation.");
    }
    std::size_t combined = a.size() + b.size();
    if (combined > MAX_LEN) {
        throw std::invalid_argument("Concatenated string would exceed maximum allowed length.");
    }
    std::string out;
    out.reserve(combined);
    out.append(a);
    out.append(b);
    return out;
}

std::string reverseString(const std::string& s) {
    validateString(s);
    std::string out(s);
    std::reverse(out.begin(), out.end());
    return out;
}

std::string toUpperCaseSafe(const std::string& s) {
    validateString(s);
    std::string out(s);
    for (char& ch : out) {
        unsigned char uch = static_cast<unsigned char>(ch);
        ch = static_cast<char>(std::toupper(uch));
    }
    return out;
}

std::string safeSubstring(const std::string& s, std::size_t start, std::size_t length) {
    validateString(s);
    if (start > s.size()) {
        throw std::invalid_argument("Start index out of bounds.");
    }
    if (length > s.size() - start) {
        throw std::invalid_argument("Substring length out of bounds.");
    }
    if (length > MAX_LEN) {
        throw std::invalid_argument("Result exceeds maximum allowed length.");
    }
    return s.substr(start, length);
}

int main() {
    try {
        // Test 1: Copy
        std::cout << "Copy: " << copyString("Hello") << "\n";
        // Test 2: Concat
        std::cout << "Concat: " << concatStrings(std::string("Hello"), std::string(" World")) << "\n";
        // Test 3: Reverse
        std::cout << "Reverse: " << reverseString("RaceCar") << "\n";
        // Test 4: To Upper
        std::cout << "Upper: " << toUpperCaseSafe("security123") << "\n";
        // Test 5: Safe Substring
        std::cout << "Substring: " << safeSubstring("abcdef", 2, 3) << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}