
#include <iostream>
#include <string>
#include <vector>

void printArgumentAtIndex(const std::vector<std::string>& args, const std::string& indexStr) {
    int index = std::stoi(indexStr);
    if (index >= 0 && index < args.size()) {
        std::cout << args[index] << std::endl;
    } else {
        std::cout << "Index out of bounds" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // Test case 1
    std::vector<std::string> test1 = {"program", "2", "hello", "world", "cpp"};
    std::cout << "Test 1: ";
    printArgumentAtIndex(test1, test1[1]);
    
    // Test case 2
    std::vector<std::string> test2 = {"program", "0", "first", "second", "third"};
    std::cout << "Test 2: ";
    printArgumentAtIndex(test2, test2[1]);
    
    // Test case 3
    std::vector<std::string> test3 = {"program", "3", "alpha", "beta", "gamma", "delta"};
    std::cout << "Test 3: ";
    printArgumentAtIndex(test3, test3[1]);
    
    // Test case 4
    std::vector<std::string> test4 = {"program", "1", "testing"};
    std::cout << "Test 4: ";
    printArgumentAtIndex(test4, test4[1]);
    
    // Test case 5
    std::vector<std::string> test5 = {"program", "5", "one", "two", "three"};
    std::cout << "Test 5: ";
    printArgumentAtIndex(test5, test5[1]);
    
    return 0;
}
