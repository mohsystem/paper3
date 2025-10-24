#include <iostream>
#include <vector>
#include <string>

std::string likes(const std::vector<std::string>& names) {
    size_t n = names.size();
    if (n == 0) return "no one likes this";
    if (n == 1) return names[0] + " likes this";
    if (n == 2) return names[0] + " and " + names[1] + " like this";
    if (n == 3) return names[0] + ", " + names[1] + " and " + names[2] + " like this";
    return names[0] + ", " + names[1] + " and " + std::to_string(n - 2) + " others like this";
}

int main() {
    std::cout << likes(std::vector<std::string>{}) << "\n";
    std::cout << likes(std::vector<std::string>{"Peter"}) << "\n";
    std::cout << likes(std::vector<std::string>{"Jacob", "Alex"}) << "\n";
    std::cout << likes(std::vector<std::string>{"Max", "John", "Mark"}) << "\n";
    std::cout << likes(std::vector<std::string>{"Alex", "Jacob", "Mark", "Max", "Lily"}) << "\n";
    return 0;
}