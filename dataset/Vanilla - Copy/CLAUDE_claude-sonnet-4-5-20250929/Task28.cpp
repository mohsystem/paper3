
#include <iostream>
#include <vector>
#include <string>

std::string whoLikesIt(const std::vector<std::string>& names) {
    int size = names.size();
    
    if (size == 0) {
        return "no one likes this";
    } else if (size == 1) {
        return names[0] + " likes this";
    } else if (size == 2) {
        return names[0] + " and " + names[1] + " like this";
    } else if (size == 3) {
        return names[0] + ", " + names[1] + " and " + names[2] + " like this";
    } else {
        int others = size - 2;
        return names[0] + ", " + names[1] + " and " + std::to_string(others) + " others like this";
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
