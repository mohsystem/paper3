
#include <iostream>
#include <random>

int generateRandomNumber() {
    // Use random_device and mt19937 for secure random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1001, INT_MAX);
    
    return dis(gen);
}

int main() {
    // Test case 1
    int result1 = generateRandomNumber();
    std::cout << "Test 1 - Random number: " << result1 << " (Should be > 1000)" << std::endl;
    
    // Test case 2
    int result2 = generateRandomNumber();
    std::cout << "Test 2 - Random number: " << result2 << " (Should be > 1000)" << std::endl;
    
    // Test case 3
    int result3 = generateRandomNumber();
    std::cout << "Test 3 - Random number: " << result3 << " (Should be > 1000)" << std::endl;
    
    // Test case 4
    int result4 = generateRandomNumber();
    std::cout << "Test 4 - Random number: " << result4 << " (Should be > 1000)" << std::endl;
    
    // Test case 5
    int result5 = generateRandomNumber();
    std::cout << "Test 5 - Random number: " << result5 << " (Should be > 1000)" << std::endl;
    
    return 0;
}
