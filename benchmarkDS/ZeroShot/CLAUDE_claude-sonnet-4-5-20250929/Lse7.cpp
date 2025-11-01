
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

void processCommandLineArguments(const std::vector<std::string>& args, int index) {
    if (args.empty()) {
        std::cout << "No arguments provided" << std::endl;
        return;
    }
    
    if (index < 0 || index >= static_cast<int>(args.size())) {
        std::cout << "Index out of bounds" << std::endl;
        return;
    }
    
    std::cout << args[index] << std::endl;
}

int main() {
    // Test case 1: Normal case
    std::vector<std::string> test1 = {"arg0", "2", "arg2", "arg3"};
    std::cout << "Test 1:" << std::endl;
    int index1 = std::stoi(test1[1]);
    processCommandLineArguments(test1, index1);
    
    // Test case 2: Index at boundary
    std::vector<std::string> test2 = {"arg0", "0", "arg2"};
    std::cout << "\\nTest 2:" << std::endl;
    int index2 = std::stoi(test2[1]);
    processCommandLineArguments(test2, index2);
    
    // Test case 3: Last element
    std::vector<std::string> test3 = {"arg0", "3", "arg2", "arg3"};
    std::cout << "\\nTest 3:" << std::endl;
    int index3 = std::stoi(test3[1]);
    processCommandLineArguments(test3, index3);
    
    // Test case 4: Out of bounds
    std::vector<std::string> test4 = {"arg0", "5", "arg2"};
    std::cout << "\\nTest 4:" << std::endl;
    int index4 = std::stoi(test4[1]);
    processCommandLineArguments(test4, index4);
    
    // Test case 5: Empty array handling
    std::vector<std::string> test5 = {};
    std::cout << "\\nTest 5:" << std::endl;
    processCommandLineArguments(test5, 0);
    
    return 0;
}
