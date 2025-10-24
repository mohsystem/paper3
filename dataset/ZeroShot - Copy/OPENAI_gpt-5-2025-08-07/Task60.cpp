#include <iostream>
#include <string>
#include <vector>

std::string expandedForm(unsigned long long n) {
    if (n == 0ULL) return "";
    std::string s = std::to_string(n);
    std::string result;
    bool first = true;
    size_t L = s.size();
    for (size_t i = 0; i < L; ++i) {
        char ch = s[i];
        if (ch != '0') {
            if (!first) result += " + ";
            result.push_back(ch);
            size_t zeros = L - 1 - i;
            result.append(zeros, '0');
            first = false;
        }
    }
    return result;
}

int main() {
    unsigned long long tests[] = {12ULL, 42ULL, 70304ULL, 900000ULL, 10501ULL};
    for (auto t : tests) {
        std::cout << expandedForm(t) << std::endl;
    }
    return 0;
}