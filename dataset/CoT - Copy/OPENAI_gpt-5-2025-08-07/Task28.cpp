#include <iostream>
#include <string>
#include <vector>

std::string likes(const std::vector<std::string>& names) {
    auto safe = [](const std::string& s) -> std::string {
        return s;
    };
    if (names.empty()) {
        return "no one likes this";
    }
    size_t n = names.size();
    const std::string& a = safe(names[0]);
    if (n == 1) {
        return a + " likes this";
    } else if (n == 2) {
        const std::string& b = safe(names[1]);
        return a + " and " + b + " like this";
    } else if (n == 3) {
        const std::string& b = safe(names[1]);
        const std::string& c = safe(names[2]);
        return a + ", " + b + " and " + c + " like this";
    } else {
        const std::string& b = safe(names[1]);
        return a + ", " + b + " and " + std::to_string(n - 2) + " others like this";
    }
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {},
        {"Peter"},
        {"Jacob", "Alex"},
        {"Max", "John", "Mark"},
        {"Alex", "Jacob", "Mark", "Max"}
    };
    for (const auto& t : tests) {
        std::cout << "[";
        for (size_t i = 0; i < t.size(); ++i) {
            std::cout << t[i];
            if (i + 1 < t.size()) std::cout << ", ";
        }
        std::cout << "] --> " << likes(t) << "\n";
    }
    return 0;
}