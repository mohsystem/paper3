#include <iostream>
#include <vector>
#include <string>

std::string getValueAtIndexFromArgs(const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        return std::string();
    }
    long long idx = 0;
    try {
        size_t pos = 0;
        idx = std::stoll(argv[1], &pos, 10);
        if (pos != argv[1].size()) {
            return std::string();
        }
    } catch (...) {
        return std::string();
    }
    if (idx < 0 || static_cast<size_t>(idx) >= argv.size()) {
        return std::string();
    }
    return argv[static_cast<size_t>(idx)];
}

int main() {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {"prog", "2", "a", "b", "c"},   // expects "a"
        {"prog", "0"},                  // expects "prog"
        {"prog", "10", "x"},            // expects "" (out of range)
        {"prog", "-1", "x"},            // expects "" (negative index)
        {"prog", "notanint", "x"}       // expects "" (parse error)
    };

    for (const auto& t : tests) {
        std::string res = getValueAtIndexFromArgs(t);
        if (res.empty()) {
            std::cout << "" << std::endl;
        } else {
            std::cout << res << std::endl;
        }
    }
    return 0;
}