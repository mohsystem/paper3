
#include <iostream>
#include <cstdlib>
#include <string>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test 1: " << addThousand(5) << " (Expected: 1005)" << std::endl;
    std::cout << "Test 2: " << addThousand(0) << " (Expected: 1000)" << std::endl;
    std::cout << "Test 3: " << addThousand(-500) << " (Expected: 500)" << std::endl;
    std::cout << "Test 4: " << addThousand(2000) << " (Expected: 3000)" << std::endl;
    std::cout << "Test 5: " << addThousand(-1000) << " (Expected: 0)" << std::endl;
    
    // Command line argument handling
    if (argc > 1) {
        try {
            int value = std::stoi(argv[1]);
            std::cout << "Command line input: " << value << " -> Result: " << addThousand(value) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Invalid input. Please provide a valid integer." << std::endl;
        }
    }
    
    return 0;
}
