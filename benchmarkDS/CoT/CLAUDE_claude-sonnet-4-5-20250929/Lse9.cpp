
#include <iostream>
#include <string>
#include <vector>

std::string getArgumentAtIndex(const std::vector<std::string>& args, int index) {
    // Validate index is non-negative
    if (index < 0) {
        return "Error: Index cannot be negative";
    }
    
    // Check if index is within bounds
    if (index >= static_cast<int>(args.size())) {
        return "Error: Index out of bounds";
    }
    
    return args[index];
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    std::vector<std::string> test1 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    std::cout << getArgumentAtIndex(test1, 1) << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::vector<std::string> test2 = {"arg0", "arg1", "arg2", "arg3", "arg4"};
    std::cout << getArgumentAtIndex(test2, 3) << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::vector<std::string> test3 = {"arg0", "arg1"};
    std::cout << getArgumentAtIndex(test3, 0) << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::vector<std::string> test4 = {"arg0", "arg1", "arg2"};
    std::cout << getArgumentAtIndex(test4, 5) << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::vector<std::string> test5 = {"arg0", "arg1", "arg2"};
    std::cout << getArgumentAtIndex(test5, -1) << std::endl;
    
    // Real command line usage
    if (argc > 1) {
        try {
            int index = std::stoi(argv[1]);
            std::vector<std::string> args;
            for (int i = 0; i < argc; i++) {
                args.push_back(argv[i]);
            }
            std::string result = getArgumentAtIndex(args, index);
            std::cout << "\\nCommand line result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: First argument must be a valid integer" << std::endl;
        }
    }
    
    return 0;
}
