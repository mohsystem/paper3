#include <iostream>
#include <string>
#include <cctype>
#include <climits>

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

// Securely tries to parse a non-negative integer from a string. Returns true on success.
bool tryParseNonNegativeInt(const std::string& s, int& out) {
    std::string t = trim(s);
    if (t.empty()) return false;
    size_t i = 0;
    if (t[0] == '+') {
        if (t.size() == 1) return false;
        i = 1;
    } else if (t[0] == '-') {
        return false;
    }
    long long val = 0;
    for (; i < t.size(); ++i) {
        char c = t[i];
        if (c < '0' || c > '9') return false;
        val = val * 10 + (c - '0');
        if (val > INT_MAX) return false;
    }
    out = static_cast<int>(val);
    return true;
}

// Returns pointer to the argument at the specified index, or nullptr if invalid.
const char* getArgAtIndex(int argc, const char* argv[], const std::string& indexStr) {
    int idx = -1;
    if (!tryParseNonNegativeInt(indexStr, idx)) return nullptr;
    if (idx < 0 || idx >= argc) return nullptr;
    return argv[idx];
}

// Prints the argument at the specified index if valid.
void printArgAtIndex(int argc, const char* argv[], const std::string& indexStr) {
    const char* v = getArgAtIndex(argc, argv, indexStr);
    if (v != nullptr) {
        std::cout << v << std::endl;
    }
}

int main(int argc, const char* argv[]) {
    // If run with CLI arguments, use the second argument as index, and print the argument at that index.
    if (argc > 1) {
        printArgAtIndex(argc, argv, std::string(argv[1]));
    }

    // 5 test cases
    {
        const char* t1[] = {"a0", "2", "x0", "x1", "x2"};       // prints x0
        printArgAtIndex(5, t1, t1[1]);
    }
    {
        const char* t2[] = {"first", "0", "z"};                 // prints first
        printArgAtIndex(3, t2, t2[1]);
    }
    {
        const char* t3[] = {"foo", "3", "aa", "bb", "cc", "dd"}; // prints dd
        printArgAtIndex(6, t3, t3[1]);
    }
    {
        const char* t4[] = {"unused", "-1", "val"};             // prints nothing (invalid)
        printArgAtIndex(3, t4, t4[1]);
    }
    {
        const char* t5[] = {"hello", "abc", "p", "q"};          // prints nothing (invalid)
        printArgAtIndex(4, t5, t5[1]);
    }

    return 0;
}