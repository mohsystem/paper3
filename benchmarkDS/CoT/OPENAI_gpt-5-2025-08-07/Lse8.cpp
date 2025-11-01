// Chain-of-Through process in code generation:
// 1) Problem understanding: Given argv (vector<string>), return argv[stoi(argv[1])].
// 2) Security requirements: Validate presence of argv[1], integer conversion, and bounds.
// 3) Secure coding generation: Use safe parsing and checks; avoid UB.
// 4) Code review: Guard all failure modes and avoid unchecked indexing.
// 5) Secure code output: Final safe implementation with tests.

#include <iostream>
#include <vector>
#include <string>
#include <limits>

std::string getValueAtIndex(const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        return std::string();
    }
    const std::string& idxStr = argv[1];
    if (idxStr.empty()) {
        return std::string();
    }
    // Safe parse to long long
    try {
        size_t pos = 0;
        long long idx = std::stoll(idxStr, &pos, 10);
        if (pos != idxStr.size()) {
            return std::string();
        }
        if (idx < 0 || static_cast<size_t>(idx) >= argv.size()) {
            return std::string();
        }
        return argv[static_cast<size_t>(idx)];
    } catch (...) {
        return std::string();
    }
}

int main() {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {"prog", "2", "alpha", "beta", "gamma"}, // expect "alpha"
        {"prog", "0", "alpha"},                  // expect "prog"
        {"prog", "5", "a", "b"},                 // invalid -> ""
        {"prog", "-1", "x", "y"},                // invalid -> ""
        {"prog", "notint", "x", "y"}             // invalid -> ""
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string res = getValueAtIndex(tests[i]);
        std::cout << "Test " << (i + 1) << ": " << (res.empty() ? "(empty)" : res) << "\n";
    }
    return 0;
}