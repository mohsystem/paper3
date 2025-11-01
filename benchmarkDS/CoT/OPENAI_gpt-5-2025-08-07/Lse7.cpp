// Chain-of-Through process:
// 1) Problem understanding: Safe parse index from string and print argv[INDEX] where INDEX is argv[1].
// 2) Security requirements: Validate input, prevent overflow, check bounds.
// 3) Secure coding generation: Implement robust parsing and range checks.
// 4) Code review: Ensure no UB, no out-of-range access, proper handling of invalid input.
// 5) Secure code output: Final code with error handling that prints "ERROR" on failure.

#include <iostream>
#include <string>
#include <vector>
#include <climits>
#include <cctype>

static bool safeParseIndex(const std::string& s, int& out) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    if (start >= end) return false;

    size_t i = start;
    if (s[i] == '+') {
        ++i;
        if (i >= end) return false;
    } else if (s[i] == '-') {
        return false;
    }

    long long val = 0;
    for (; i < end; ++i) {
        char c = s[i];
        if (c < '0' || c > '9') return false;
        val = val * 10 + (c - '0');
        if (val > INT_MAX) return false;
    }
    out = static_cast<int>(val);
    return true;
}

static const std::string* getArgAtIndex(const std::vector<std::string>& argv, const std::string& indexStr) {
    int idx = -1;
    if (!safeParseIndex(indexStr, idx)) return nullptr;
    if (idx < 0 || static_cast<size_t>(idx) >= argv.size()) return nullptr;
    return &argv[static_cast<size_t>(idx)];
}

static const std::string* printAtIndex(const std::vector<std::string>& argv, const std::string& indexStr) {
    const std::string* val = getArgAtIndex(argv, indexStr);
    if (val) {
        std::cout << *val << "\n";
        return val;
    } else {
        std::cout << "ERROR\n";
        return nullptr;
    }
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"a", "2", "alpha", "beta", "gamma"}, // prints alpha
        {"first", "0", "hello"},              // prints first
        {"cat", "3", "a", "b", "c", "d"},     // prints d
        {"oops", "notnum", "foo"},            // ERROR
        {"few", "9", "only", "two"}           // ERROR
    };
    for (const auto& t : tests) {
        std::string idxStr = (t.size() > 1) ? t[1] : std::string();
        printAtIndex(t, idxStr);
    }
    return 0;
}