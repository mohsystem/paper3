#include <iostream>
#include <string>
#include <vector>

static std::string likes_display(const std::vector<std::string>& names) {
    const std::size_t n = names.size();
    switch (n) {
        case 0:
            return "no one likes this";
        case 1:
            return names[0] + " likes this";
        case 2:
            return names[0] + " and " + names[1] + " like this";
        case 3:
            return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        default:
            return names[0] + ", " + names[1] + " and " + std::to_string(n - 2) + " others like this";
    }
}

int main() {
    // 5 test cases
    std::vector<std::vector<std::string>> tests = {
        {},                                              // []
        {"Peter"},                                       // ["Peter"]
        {"Jacob", "Alex"},                               // ["Jacob", "Alex"]
        {"Max", "John", "Mark"},                         // ["Max", "John", "Mark"]
        {"Alex", "Jacob", "Mark", "Max"}                 // ["Alex", "Jacob", "Mark", "Max"]
    };

    for (const auto& t : tests) {
        std::cout << likes_display(t) << "\n";
    }

    return 0;
}