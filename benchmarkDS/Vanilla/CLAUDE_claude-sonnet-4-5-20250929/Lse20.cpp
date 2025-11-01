
#include <iostream>
#include <cstdlib>

const int OFFSET = 1000;

int addOffset(int value) {
    return value + OFFSET;
}

int main(int argc, char* argv[]) {
    // Test case 1
    std::cout << "Test 1: Input = 5, Output = " << addOffset(5) << std::endl;
    
    // Test case 2
    std::cout << "Test 2: Input = 100, Output = " << addOffset(100) << std::endl;
    
    // Test case 3
    std::cout << "Test 3: Input = -500, Output = " << addOffset(-500) << std::endl;
    
    // Test case 4
    std::cout << "Test 4: Input = 0, Output = " << addOffset(0) << std::endl;
    
    // Test case 5
    std::cout << "Test 5: Input = 2500, Output = " << addOffset(2500) << std::endl;
    
    // If command line argument is provided
    if (argc > 1) {
        int inputValue = std::atoi(argv[1]);
        int result = addOffset(inputValue);
        std::cout << "Command line input: " << inputValue << ", Result: " << result << std::endl;
    }
    
    return 0;
}
