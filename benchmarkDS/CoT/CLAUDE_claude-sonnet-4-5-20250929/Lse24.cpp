
#include <iostream>
#include <random>
#include <cmath>

class Lse24 {
public:
    static void generateAndCalculateDifference() {
        // Use random_device and mt19937 for secure random number generation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(10, 1000);
        
        // Generate random integers between 10 and 1000 (inclusive)
        int num1 = dis(gen);
        int num2 = dis(gen);
        
        // Calculate the absolute difference
        int difference = std::abs(num1 - num2);
        
        // Print the results
        std::cout << "First number: " << num1 << std::endl;
        std::cout << "Second number: " << num2 << std::endl;
        std::cout << "Difference: " << difference << std::endl;
    }
};

int main() {
    std::cout << "Test Case 1:" << std::endl;
    Lse24::generateAndCalculateDifference();
    std::cout << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    Lse24::generateAndCalculateDifference();
    std::cout << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    Lse24::generateAndCalculateDifference();
    std::cout << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    Lse24::generateAndCalculateDifference();
    std::cout << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    Lse24::generateAndCalculateDifference();
    
    return 0;
}
