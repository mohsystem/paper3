#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <cctype>
#include <limits>

std::string value_at_index(const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        throw std::invalid_argument("insufficient arguments");
    }
    const std::string& idxStr = argv[1];
    if (idxStr.empty() || idxStr.size() > 10) {
        throw std::invalid_argument("invalid index");
    }
    for (char c : idxStr) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            throw std::invalid_argument("invalid index");
        }
    }
    unsigned long long idx = 0;
    try {
        idx = std::stoull(idxStr);
    } catch (...) {
        throw std::invalid_argument("invalid index");
    }
    if (idx > static_cast<unsigned long long>(std::numeric_limits<int>::max())) {
        throw std::invalid_argument("invalid index");
    }
    int i = static_cast<int>(idx);
    if (i < 0 || static_cast<size_t>(i) >= argv.size()) {
        throw std::out_of_range("index out of range");
    }
    return argv[static_cast<size_t>(i)];
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"prog", "0"},
        {"app", "2", "x", "y", "z"},
        {"cmd", "4", "A", "B", "C", "D", "E"},
        {"tool", "-1", "val"},
        {"t", "10", "a", "b"}
    };

    for (size_t t = 0; t < tests.size(); ++t) {
        try {
            std::string res = value_at_index(tests[t]);
            std::cout << "Test " << (t + 1) << " -> " << res << "\n";
        } catch (const std::exception& ex) {
            std::cout << "Test " << (t + 1) << " -> error: " << ex.what() << "\n";
        }
    }

    return 0;
}