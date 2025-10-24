#include <iostream>
#include <string>
#include <algorithm>

bool solution(const std::string& s, const std::string& ending) {
    if (ending.size() > s.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), s.rbegin());
}

int main() {
    std::pair<std::string, std::string> tests[] = {
        {"abc", "bc"},
        {"abc", "d"},
        {"", ""},
        {"abc", ""},
        {"a", "ab"}
    };
    for (const auto& t : tests) {
        std::cout << (solution(t.first, t.second) ? "true" : "false") << std::endl;
    }
    return 0;
}