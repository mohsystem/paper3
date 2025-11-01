
#include <iostream>
#include <cstdlib>
#include <string>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test 1: " << addThousand(500) << std::endl;      // Expected: 1500
    std::cout << "Test 2: " << addThousand(0) << std::endl;        // Expected: 1000
    std::cout << "Test 3: " << addThousand(-100) << std::endl;     // Expected: 900
    std::cout << "Test 4: " << addThousand(2500) << std::endl;     // Expected: 3500
    std::cout << "Test 5: " << addThousand(-1000) << std::endl;    // Expected: 0
    
    // Command line argument handling
    if (argc > 1) {
        try {
            int value = std::stoi(argv[1]);
            std::cout << "Command line result: " << addThousand(value) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid input. Please provide a valid integer." << std::endl;
        }
    }
    
    return 0;
}
