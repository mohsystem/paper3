
#include <iostream>
#include <vector>
#include <string>

std::string whoLikesIt(const std::vector<std::string>& names) {
    if (names.empty()) {
        return "no one likes this";
    }
    
    switch (names.size()) {
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
    // Test case 1
    std::vector<std::string> test1 = {};
    std::cout << whoLikesIt(test1) << std::endl;
    
    // Test case 2
    std::vector<std::string> test2 = {"Peter"};
    std::cout << whoLikesIt(test2) << std::endl;
    
    // Test case 3
    std::vector<std::string> test3 = {"Jacob", "Alex"};
    std::cout << whoLikesIt(test3) << std::endl;
    
    // Test case 4
    std::vector<std::string> test4 = {"Max", "John", "Mark"};
    std::cout << whoLikesIt(test4) << std::endl;
    
    // Test case 5
    std::vector<std::string> test5 = {"Alex", "Jacob", "Mark", "Max"};
    std::cout << whoLikesIt(test5) << std::endl;
    
    return 0;
}
