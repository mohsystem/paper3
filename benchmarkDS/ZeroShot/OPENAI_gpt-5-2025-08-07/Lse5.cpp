#include <iostream>
#include <vector>
#include <string>
#include <limits>

bool getElementAtIndex(const std::vector<std::string>& elements, const std::string& indexStr, std::string& outElement) {
    // Validate input
    if (indexStr.empty()) return false;

    // Trim (simple)
    size_t start = 0, end = indexStr.size();
    while (start < end && std::isspace(static_cast<unsigned char>(indexStr[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(indexStr[end - 1]))) --end;
    if (start >= end) return false;

    // Disallow negative, allow optional leading '+'
    size_t pos = start;
    if (indexStr[pos] == '+') {
        ++pos;
        if (pos >= end) return false;
    } else if (indexStr[pos] == '-') {
        return false;
    }

    // Ensure all remaining are digits
    for (size_t i = pos; i < end; ++i) {
        if (!std::isdigit(static_cast<unsigned char>(indexStr[i]))) return false;
    }

    try {
        size_t convPos = 0;
        long long val = std::stoll(indexStr.substr(start, end - start), &convPos, 10);
        if (convPos != (end - start)) return false;
        if (val < 0) return false;
        if (val > static_cast<long long>(std::numeric_limits<int>::max())) return false;
        size_t idx = static_cast<size_t>(val);
        if (idx >= elements.size()) return false;
        outElement = elements[idx];
        return true;
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    std::vector<std::string> elements = {"alpha", "bravo", "charlie", "delta", "echo"};

    // Process command-line argument if provided
    if (argc >= 2) {
        std::string out;
        if (getElementAtIndex(elements, argv[1], out)) {
            std::cout << out << "\n";
        } else {
            std::cerr << "Invalid input or index out of bounds.\n";
        }
    }

    // Five test cases
    std::vector<std::string> tests = {"0", "2", "4", "-1", "abc"};
    for (const auto& t : tests) {
        std::string out;
        if (getElementAtIndex(elements, t, out)) {
            std::cout << "Test index \"" << t << "\": " << out << "\n";
        } else {
            std::cout << "Test index \"" << t << "\": Invalid\n";
        }
    }
    return 0;
}