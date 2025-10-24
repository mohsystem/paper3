#include <iostream>
#include <vector>
#include <string>

std::string expandedForm(long long num) {
    std::string s = std::to_string(num);
    std::string result;
    for (size_t i = 0; i < s.size(); ++i) {
        int d = s[i] - '0';
        if (d == 0) continue;
        long long place = 1;
        for (size_t j = i + 1; j < s.size(); ++j) place *= 10;
        long long val = d * place;
        if (!result.empty()) result += " + ";
        result += std::to_string(val);
    }
    return result;
}

int main() {
    std::vector<long long> tests = {12, 42, 70304, 9000000, 105};
    for (auto t : tests) {
        std::cout << t << " -> " << expandedForm(t) << std::endl;
    }
    return 0;
}