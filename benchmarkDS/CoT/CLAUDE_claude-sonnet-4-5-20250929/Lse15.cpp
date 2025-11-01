
#include <iostream>
#include <string>
#include <sstream>
#include <limits>

class Lse15 {
private:
    static const int id_sequence[];
    static const int ARRAY_SIZE = 10;
    
public:
    static int* getValueAtIndex(int index) {
        // Validate index bounds
        if (index < 0 || index >= ARRAY_SIZE) {
            std::cout << "Error: Index out of bounds. Valid range: 0 to " 
                      << (ARRAY_SIZE - 1) << std::endl;
            return nullptr;
        }
        return const_cast<int*>(&id_sequence[index]);
    }
    
    static int* getValueWithPrompt() {
        std::cout << "Enter an index value: ";
        std::string input;
        std::getline(std::cin, input);
        
        // Validate input is not empty
        if (input.empty()) {
            std::cout << "Error: Empty input" << std::endl;
            return nullptr;
        }
        
        // Try to parse integer
        std::stringstream ss(input);
        int index;
        if (!(ss >> index) || !(ss.eof())) {
            std::cout << "Error: Invalid input. Please enter a valid integer." << std::endl;
            return nullptr;
        }
        
        return getValueAtIndex(index);
    }
    
    static void runTests() {
        std::cout << "Test Case 1: Valid index 0" << std::endl;
        int* result1 = getValueAtIndex(0);
        std::cout << "Result: " << (result1 ? std::to_string(*result1) : "null") << "\\n" << std::endl;
        
        std::cout << "Test Case 2: Valid index 5" << std::endl;
        int* result2 = getValueAtIndex(5);
        std::cout << "Result: " << (result2 ? std::to_string(*result2) : "null") << "\\n" << std::endl;
        
        std::cout << "Test Case 3: Valid index 9 (last element)" << std::endl;
        int* result3 = getValueAtIndex(9);
        std::cout << "Result: " << (result3 ? std::to_string(*result3) : "null") << "\\n" << std::endl;
        
        std::cout << "Test Case 4: Invalid negative index -1" << std::endl;
        int* result4 = getValueAtIndex(-1);
        std::cout << "Result: " << (result4 ? std::to_string(*result4) : "null") << "\\n" << std::endl;
        
        std::cout << "Test Case 5: Invalid index 10 (out of bounds)" << std::endl;
        int* result5 = getValueAtIndex(10);
        std::cout << "Result: " << (result5 ? std::to_string(*result5) : "null") << "\\n" << std::endl;
    }
};

const int Lse15::id_sequence[] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};

int main() {
    Lse15::runTests();
    return 0;
}
