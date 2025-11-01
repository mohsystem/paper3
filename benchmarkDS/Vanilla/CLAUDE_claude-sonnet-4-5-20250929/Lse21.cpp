
#include <iostream>
#include <cstdlib>

int addThousand(int value) {
    int result = value + 1000;
    std::cout << result << std::endl;
    return 0;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test case 1:" << std::endl;
    addThousand(5);
    
    std::cout << "\\nTest case 2:" << std::endl;
    addThousand(100);
    
    std::cout << "\\nTest case 3:" << std::endl;
    addThousand(-50);
    
    std::cout << "\\nTest case 4:" << std::endl;
    addThousand(0);
    
    std::cout << "\\nTest case 5:" << std::endl;
    addThousand(9999);
    
    // If command line argument is provided
    if (argc > 1) {
        int value = std::atoi(argv[1]);
        std::cout << "\\nCommand line input:" << std::endl;
        addThousand(value);
    }
    
    return 0;
}
