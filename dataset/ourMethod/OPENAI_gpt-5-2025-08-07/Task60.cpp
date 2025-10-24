#include <iostream>
#include <string>
#include <vector>

static std::string expandedForm(unsigned long long n) {
    if (n == 0ULL) {
        return std::string("0");
    }
    const std::string s = std::to_string(n);
    std::vector<std::string> parts;
    const std::size_t len = s.size();
    parts.reserve(len);

    for (std::size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (c == '0') continue;
        std::string term;
        term.push_back(c);
        std::size_t zeros = len - i - 1;
        term.append(zeros, '0');
        parts.push_back(term);
    }

    if (parts.empty()) {
        return std::string("0");
    }

    std::string result;
    // Compute final length to reserve (optional but safe)
    std::size_t total_len = 0;
    for (std::size_t i = 0; i < parts.size(); ++i) {
        total_len += parts[i].size();
        if (i + 1 < parts.size()) total_len += 3; // " + "
    }
    result.reserve(total_len);

    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += " + ";
        result += parts[i];
    }
    return result;
}

int main() {
    std::vector<unsigned long long> tests = {
        12ULL,       // "10 + 2"
        42ULL,       // "40 + 2"
        70304ULL,    // "70000 + 300 + 4"
        9000000ULL,  // "9000000"
        105ULL       // "100 + 5"
    };

    for (auto n : tests) {
        std::string out = expandedForm(n);
        std::cout << n << " -> " << out << std::endl;
    }
    return 0;
}