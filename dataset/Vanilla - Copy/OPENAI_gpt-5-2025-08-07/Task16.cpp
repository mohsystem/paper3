#include <iostream>
#include <string>
#include <algorithm>

bool solution(const std::string& str, const std::string& ending) {
    if (ending.size() > str.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}

int main() {
    std::pair<std::string, std::string> tests[] = {
        {"abc", "bc"},
        {"abc", "d"},
        {"hello", ""},
        {"", ""},
        {"JavaTask16", "Task16"}
    };
    std::cout << std::boolalpha;
    for (const auto& t : tests) {
        std::cout << "solution(\"" << t.first << "\", \"" << t.second << "\") -> "
                  << solution(t.first, t.second) << "\n";
    }
    return 0;
}