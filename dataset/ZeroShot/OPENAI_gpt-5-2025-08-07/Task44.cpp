#include <iostream>
#include <string>
#include <vector>

static bool isAllowed(char ch) {
    unsigned char uc = static_cast<unsigned char>(ch);
    if (uc >= 'a' && uc <= 'z') return true;
    if (uc >= 'A' && uc <= 'Z') return true;
    if (uc >= '0' && uc <= '9') return true;
    if (ch == ' ' || ch == '\t') return true;
    switch (ch) {
        case '_':
        case '-':
        case '.':
        case ',':
        case ':':
        case '@':
            return true;
        default:
            return false;
    }
}

std::string processInput(const std::string& input) {
    if (input.size() > 256) {
        return "Invalid input";
    }
    for (char ch : input) {
        if (!isAllowed(ch)) {
            return "Invalid input";
        }
    }
    std::string sanitized;
    sanitized.reserve(input.size());
    bool lastWasSpace = true; // trim leading and collapse
    for (char ch : input) {
        if (ch == ' ' || ch == '\t') {
            if (!lastWasSpace) {
                sanitized.push_back(' ');
                lastWasSpace = true;
            }
        } else {
            sanitized.push_back(ch);
            lastWasSpace = false;
        }
    }
    if (!sanitized.empty() && sanitized.back() == ' ') {
        sanitized.pop_back();
    }
    if (sanitized.empty()) {
        return "Invalid input";
    }
    size_t digitCount = 0;
    for (char ch : sanitized) {
        if (ch >= '0' && ch <= '9') ++digitCount;
    }
    std::string reversed = sanitized;
    std::reverse(reversed.begin(), reversed.end());
    return "sanitized=" + sanitized + "; length=" + std::to_string(sanitized.size()) +
           "; digits=" + std::to_string(digitCount) + "; reversed=" + reversed;
}

int main() {
    std::vector<std::string> tests = {
        "Hello World",
        "  Multiple    spaces   123  ",
        "Invalid!Chars#",
        "A_Very-Long.String,With:Allowed@Chars",
        ""
    };
    for (const auto& t : tests) {
        std::cout << "Input: \"" << t << "\" -> " << processInput(t) << std::endl;
    }
    return 0;
}