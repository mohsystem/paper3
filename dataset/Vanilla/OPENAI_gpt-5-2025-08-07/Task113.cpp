#include <iostream>
#include <vector>
#include <string>

std::string concatStrings(const std::vector<std::string>& parts) {
    size_t total = 0;
    for (const auto& s : parts) total += s.size();
    std::string result;
    result.reserve(total);
    for (const auto& s : parts) result += s;
    return result;
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"Hello", " ", "World"},
        {"", "abc", "", "123"},
        {"Join", "-", "these", "-", "together"},
        {"multi", "", "ple", " ", "strings"},
        {"Unicode:", " ", "😊", " ", "测试"}
    };

    for (const auto& t : tests) {
        std::cout << concatStrings(t) << std::endl;
    }
    return 0;
}