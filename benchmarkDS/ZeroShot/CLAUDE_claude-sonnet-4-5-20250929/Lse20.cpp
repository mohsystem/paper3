
#include <iostream>
#include <cstdlib>
#include <string>

const int OFFSET = 1000;

int addOffset(int value) {
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1: " << addOffset(100) << " (Expected: 1100)" << std::endl;
    std::cout << "Test Case 2: " << addOffset(0) << " (Expected: 1000)" << std::endl;
    std::cout << "Test Case 3: " << addOffset(-500) << " (Expected: 500)" << std::endl;
    std::cout << "Test Case 4: " << addOffset(2500) << " (Expected: 3500)" << std::endl;
    std::cout << "Test Case 5: " << addOffset(-1000) << " (Expected: 0)" << std::endl;
    
    // Read from command line if argument provided
    if (argc > 1) {
        try {
            int inputValue = std::stoi(argv[1]);
            int result = addOffset(inputValue);
            std::cout << "\\nCommand line input: " << inputValue << std::endl;
            std::cout << "Result after adding offset: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: Invalid number format" << std::endl;
        }
    }
    
    return 0;
}
