
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>

class Lse8 {
public:
    static std::string getArgumentAtIndex(const std::vector<std::string>& args) {
        // Security check: Validate arguments vector
        if (args.empty() || args.size() < 2) {
            return "Error: Insufficient arguments. Need at least 2 arguments.";
        }
        
        try {
            // Security check: Safely parse the index
            int index;
            std::stringstream ss(args[1]);
            
            if (!(ss >> index) || !ss.eof()) {
                return "Error: Invalid index format. Index must be an integer.";
            }
            
            // Security check: Validate index bounds
            if (index < 0 || static_cast<size_t>(index) >= args.size()) {
                return "Error: Index out of bounds. Valid range: 0 to " + 
                       std::to_string(args.size() - 1);
            }
            
            // Return the value at the specified index
            return args[index];
            
        } catch (const std::exception& e) {
            return std::string("Error: ") + e.what();
        }
    }
};

int main() {
    // Test case 1: Normal case
    std::vector<std::string> test1 = {"program", "2", "hello", "world"};
    std::cout << "Test 1: " << Lse8::getArgumentAtIndex(test1) << std::endl;
    
    // Test case 2: Index out of bounds
    std::vector<std::string> test2 = {"program", "10", "hello"};
    std::cout << "Test 2: " << Lse8::getArgumentAtIndex(test2) << std::endl;
    
    // Test case 3: Invalid index format
    std::vector<std::string> test3 = {"program", "abc", "hello"};
    std::cout << "Test 3: " << Lse8::getArgumentAtIndex(test3) << std::endl;
    
    // Test case 4: Insufficient arguments
    std::vector<std::string> test4 = {"program"};
    std::cout << "Test 4: " << Lse8::getArgumentAtIndex(test4) << std::endl;
    
    // Test case 5: Negative index
    std::vector<std::string> test5 = {"program", "-1", "hello"};
    std::cout << "Test 5: " << Lse8::getArgumentAtIndex(test5) << std::endl;
    
    return 0;
}
