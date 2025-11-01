#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>

std::string getArgAtIndex(const std::vector<std::string>& argv, int index) {
    if (index < 0 || index >= static_cast<int>(argv.size())) {
        return std::string();
    }
    return argv[index];
}

static bool parseIndex(const char* s, int& out) {
    if (s == nullptr) return false;
    errno = 0;
    char* end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return false;
    if (v < 0 || v > INT_MAX) return false;
    out = static_cast<int>(v);
    return true;
}

static void runTests() {
    std::vector<std::string> testArgv = {"prog", "1", "alpha", "beta", "gamma"};
    int tests[5] = {1, 2, 4, 0, 10};
    for (int idx : tests) {
        std::string res = getArgAtIndex(testArgv, idx);
        if (!res.empty()) {
            std::cout << res << "\n";
        } else {
            std::cout << "ERROR\n";
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        runTests();
        return 0;
    }
    int idx = -1;
    if (!parseIndex(argv[1], idx)) {
        std::cerr << "Error: invalid index\n";
        return 1;
    }
    if (idx < 0) {
        std::cerr << "Error: index must be non-negative\n";
        return 1;
    }
    // argv already includes program name at index 0
    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i] ? argv[i] : "");
    }
    std::string result = getArgAtIndex(args, idx);
    if (result.empty() && (idx < 0 || idx >= static_cast<int>(args.size()))) {
        std::cerr << "Error: index out of range\n";
        return 1;
    }
    std::cout << result << "\n";
    return 0;
}