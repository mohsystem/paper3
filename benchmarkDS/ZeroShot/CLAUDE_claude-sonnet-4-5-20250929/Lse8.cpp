
#include <iostream>
#include <string>
#include <vector>

std::string getValueAtIndex(const std::vector<std::string>& args, int index) {
    if (args.empty() || index < 0 || index >= static_cast<int>(args.size())) {
        return "";
    }
    return args[index];
}

int main() {
    // Test case 1: Normal case with valid index
    std::vector<std::string> test1 = {"arg0", "arg1", "arg2", "arg3"};
    std::cout << "Test 1: " << getValueAtIndex(test1, 1) << std::endl;
    
    // Test case 2: Index 0
    std::vector<std::string> test2 = {"first", "second", "third"};
    std::cout << "Test 2: " << getValueAtIndex(test2, 0) << std::endl;
    
    // Test case 3: Last index
    std::vector<std::string> test3 = {"a", "b", "c", "d", "e"};
    std::cout << "Test 3: " << getValueAtIndex(test3, 4) << std::endl;
    
    // Test case 4: Out of bounds index
    std::vector<std::string> test4 = {"x", "y", "z"};
    std::cout << "Test 4: " << getValueAtIndex(test4, 5) << std::endl;
    
    // Test case 5: Negative index
    std::vector<std::string> test5 = {"hello", "world"};
    std::cout << "Test 5: " << getValueAtIndex(test5, -1) << std::endl;
    
    return 0;
}
