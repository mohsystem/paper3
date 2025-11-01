
#include <iostream>
#include <cstdlib>
#include <string>

int addThousand(int value) {
    return value + 1000;
}

int main(int argc, char* argv[]) {
    // Test cases
    int testCases[] = {100, -500, 0, 9999, -1000};
    
    for (int i = 0; i < 5; i++) {
        int result = addThousand(testCases[i]);
        std::cout << "Input: " << testCases[i] << ", Output: " << result << std::endl;
    }
    
    // Command line argument handling
    if (argc > 1) {
        try {
            int value = std::stoi(argv[1]);
            int result = addThousand(value);
            std::cout << result << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid input. Please provide a valid integer." << std::endl;
            return 1;
        }
    }
    
    return 0;
}
