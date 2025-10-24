#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <limits>

bool tryParseInt(const std::string& s, int& out) {
    // Trim leading/trailing whitespace
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return false;
    size_t endpos = s.find_last_not_of(" \t\n\r\f\v");
    std::string str = s.substr(start, endpos - start + 1);

    errno = 0;
    char* end = nullptr;
    long val = std::strtol(str.c_str(), &end, 10);

    if (end == str.c_str()) return false;            // no digits
    // Allow trailing whitespace only
    while (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r' || *end == '\f' || *end == '\v') ++end;
    if (*end != '\0') return false;                  // extra non-space chars
    if (errno == ERANGE) return false;               // out of range for long
    if (val < std::numeric_limits<int>::min() || val > std::numeric_limits<int>::max()) return false;

    out = static_cast<int>(val);
    return true;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Enter value #" << i << ": ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        int result;
        if (tryParseInt(line, result)) {
            std::cout << "Parsed integer: " << result << "\n";
        } else {
            std::cout << "Invalid integer\n";
        }
    }
    return 0;
}