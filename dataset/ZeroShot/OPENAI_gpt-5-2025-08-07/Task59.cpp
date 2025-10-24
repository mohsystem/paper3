#include <iostream>
#include <string>
#include <cctype>

std::string accum(const std::string& s) {
    std::string out;
    if (!s.empty()) {
        // Reserve approximate capacity to reduce reallocations
        out.reserve((s.size() * (s.size() + 3)) / 2);
    }
    for (size_t i = 0; i < s.size(); ++i) {
        if (i > 0) out.push_back('-');
        unsigned char uc = static_cast<unsigned char>(s[i]);
        out.push_back(static_cast<char>(std::toupper(uc)));
        char lower = static_cast<char>(std::tolower(uc));
        for (size_t k = 0; k < i; ++k) out.push_back(lower);
    }
    return out;
}

int main() {
    std::string tests[] = {"abcd", "RqaEzty", "cwAt", "", "Z"};
    for (const auto& t : tests) {
        std::cout << accum(t) << std::endl;
    }
    return 0;
}