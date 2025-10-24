#include <iostream>
#include <string>
#include <vector>

std::string expandedForm(unsigned long long n) {
    if (n == 0ULL) return "0";
    std::string s = std::to_string(n);
    std::string res;
    bool first = true;
    int len = static_cast<int>(s.size());
    for (int i = 0; i < len; ++i) {
        char c = s[i];
        if (c != '0') {
            if (!first) res += " + ";
            res.push_back(c);
            res.append(static_cast<size_t>(len - i - 1), '0');
            first = false;
        }
    }
    if (first) return std::string("0");
    return res;
}

int main() {
    std::vector<unsigned long long> tests = {12ULL, 42ULL, 70304ULL, 9000000ULL, 102030ULL};
    for (auto t : tests) {
        std::cout << t << " -> " << expandedForm(t) << "\n";
    }
    return 0;
}