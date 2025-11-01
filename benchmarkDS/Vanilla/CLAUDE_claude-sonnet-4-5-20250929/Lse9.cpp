
#include <iostream>
#include <vector>
#include <string>

void printArgumentAtIndex(const std::vector<std::string>& args, int index) {
    if (index >= 0 && index < args.size()) {
        std::cout << args[index] << std::endl;
    } else {
        std::cout << "Index out of bounds" << std::endl;
    }
}

int main() {
    // Test case 1: Index 0
    std::vector<std::string> test1 = {"first", "second", "third"};
    std::cout << "Test 1 (index 0): ";
    printArgumentAtIndex(test1, 0);
    
    // Test case 2: Index 1
    std::vector<std::string> test2 = {"apple", "banana", "cherry"};
    std::cout << "Test 2 (index 1): ";
    printArgumentAtIndex(test2, 1);
    
    // Test case 3: Index 2
    std::vector<std::string> test3 = {"hello", "world", "cpp", "programming"};
    std::cout << "Test 3 (index 2): ";
    printArgumentAtIndex(test3, 2);
    
    // Test case 4: Index out of bounds
    std::vector<std::string> test4 = {"one", "two"};
    std::cout << "Test 4 (index 5): ";
    printArgumentAtIndex(test4, 5);
    
    // Test case 5: Index 3
    std::vector<std::string> test5 = {"a", "b", "c", "d", "e"};
    std::cout << "Test 5 (index 3): ";
    printArgumentAtIndex(test5, 3);
    
    return 0;
}
