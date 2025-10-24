#include <iostream>
#include <string>
#include <vector>

std::string likes(const std::vector<std::string>& names) {
    size_t n = names.size();
    auto get = [](const std::string& s) -> std::string { return s; };

    if (n == 0) return "no one likes this";
    if (n == 1) return get(names[0]) + " likes this";
    if (n == 2) return get(names[0]) + " and " + get(names[1]) + " like this";
    if (n == 3) return get(names[0]) + ", " + get(names[1]) + " and " + get(names[2]) + " like this";
    return get(names[0]) + ", " + get(names[1]) + " and " + std::to_string(n - 2) + " others like this";
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {},
        {"Peter"},
        {"Jacob", "Alex"},
        {"Max", "John", "Mark"},
        {"Alex", "Jacob", "Mark", "Max"}
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << ": " << likes(tests[i]) << std::endl;
    }
    return 0;
}