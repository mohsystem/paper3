
#include <iostream>
#include <string>
#include <vector>

std::string getArgumentAtIndex(const std::vector<std::string>& args, int index) {
    if (index < 0 || index >= static_cast<int>(args.size())) {
        return "Error: Invalid index or no argument at the specified index";
    }
    return args[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    std::vector<std::string> testArgs1 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    
    // Test case 1: Get argument at index 1
    std::cout << "Test 1 - Index 1: " << getArgumentAtIndex(testArgs1, 1) << std::endl;
    
    // Test case 2: Get argument at index 3
    std::cout << "Test 2 - Index 3: " << getArgumentAtIndex(testArgs1, 3) << std::endl;
    
    // Test case 3: Get argument at index 0
    std::cout << "Test 3 - Index 0: " << getArgumentAtIndex(testArgs1, 0) << std::endl;
    
    // Test case 4: Invalid index (out of bounds)
    std::cout << "Test 4 - Index 10: " << getArgumentAtIndex(testArgs1, 10) << std::endl;
    
    // Test case 5: Negative index
    std::cout << "Test 5 - Index -1: " << getArgumentAtIndex(testArgs1, -1) << std::endl;
    
    // If actual command line arguments are provided
    if (argc > 1) {
        try {
            int index = std::stoi(argv[1]);
            std::vector<std::string> remainingArgs;
            for (int i = 2; i < argc; i++) {
                remainingArgs.push_back(argv[i]);
            }
            std::cout << "\\nActual argument at index " << index << ": " 
                      << getArgumentAtIndex(remainingArgs, index) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: First argument must be a valid integer" << std::endl;
        }
    }
    
    return 0;
}
