
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

void processArguments(const std::vector<std::string>& args) {
    // Validate we have at least 2 arguments
    if (args.size() < 2) {
        std::cout << "Error: Insufficient arguments" << std::endl;
        return;
    }
    
    try {
        // Convert string at index 1 to integer
        int index = std::stoi(args[1]);
        
        // Validate index is within bounds
        if (index < 0 || index >= static_cast<int>(args.size())) {
            std::cout << "Error: Index out of bounds" << std::endl;
            return;
        }
        
        // Print the argument at the specified index
        std::cout << args[index] << std::endl;
        
    } catch (const std::invalid_argument&) {
        std::cout << "Error: Invalid index format" << std::endl;
    } catch (const std::out_of_range&) {
        std::cout << "Error: Index value too large" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
}

int main() {
    // Test case 1: Valid index
    std::vector<std::string> test1 = {"arg0", "2", "Hello", "World"};
    std::cout << "Test 1: ";
    processArguments(test1);
    
    // Test case 2: Index out of bounds
    std::vector<std::string> test2 = {"arg0", "10", "Hello"};
    std::cout << "Test 2: ";
    processArguments(test2);
    
    // Test case 3: Invalid index format
    std::vector<std::string> test3 = {"arg0", "abc", "Hello"};
    std::cout << "Test 3: ";
    processArguments(test3);
    
    // Test case 4: Insufficient arguments
    std::vector<std::string> test4 = {"arg0"};
    std::cout << "Test 4: ";
    processArguments(test4);
    
    // Test case 5: Access first argument
    std::vector<std::string> test5 = {"First", "0", "Second", "Third"};
    std::cout << "Test 5: ";
    processArguments(test5);
    
    return 0;
}
