#include <iostream>
#include <vector>
#include <string>

std::string whoLikesIt(const std::vector<std::string>& names) {
    switch (names.size()) {
        case 0:
            return "no one likes this";
        case 1:
            return names[0] + " likes this";
        case 2:
            return names[0] + " and " + names[1] + " like this";
        case 3:
            return names[0] + ", " + names[1] + " and " + names[2] + " like this";
        default:
            return names[0] + ", " + names[1] + " and " + std::to_string(names.size() - 2) + " others like this";
    }
}

int main() {
    // Test Case 1
    std::cout << whoLikesIt({}) << std::endl;

    // Test Case 2
    std::cout << whoLikesIt({"Peter"}) << std::endl;

    // Test Case 3
    std::cout << whoLikesIt({"Jacob", "Alex"}) << std::endl;

    // Test Case 4
    std::cout << whoLikesIt({"Max", "John", "Mark"}) << std::endl;

    // Test Case 5
    std::cout << whoLikesIt({"Alex", "Jacob", "Mark", "Max"}) << std::endl;

    return 0;
}