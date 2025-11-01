
#include <iostream>
#include <cstdlib>
#include <ctime>

void generateAndCalculateDifference() {
    // Generate first random integer between 10 and 1000
    int num1 = rand() % 991 + 10;
    
    // Generate second random integer between 10 and 1000
    int num2 = rand() % 991 + 10;
    
    // Calculate the difference between the two integers
    int difference = num1 - num2;
    
    // Print the results
    std::cout << "First number: " << num1 << std::endl;
    std::cout << "Second number: " << num2 << std::endl;
    std::cout << "Difference: " << difference << std::endl;
}

int main() {
    // Seed the random number generator
    srand(time(0));
    
    // Test case 1
    std::cout << "Test Case 1:" << std::endl;
    generateAndCalculateDifference();
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test Case 2:" << std::endl;
    generateAndCalculateDifference();
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test Case 3:" << std::endl;
    generateAndCalculateDifference();
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test Case 4:" << std::endl;
    generateAndCalculateDifference();
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test Case 5:" << std::endl;
    generateAndCalculateDifference();
    
    return 0;
}
