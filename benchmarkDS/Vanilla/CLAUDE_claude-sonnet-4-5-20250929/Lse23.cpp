
#include <iostream>
#include <cstdlib>
#include <ctime>

void generateAndPrintRandomIntegers() {
    // Generate three random integers larger than 1000
    int a = rand() % 9000 + 1001; // generates numbers from 1001 to 10000
    int b = rand() % 9000 + 1001;
    int c = rand() % 9000 + 1001;
    
    // Print the three integers
    std::cout << "a : " << a << std::endl;
    std::cout << "b : " << b << std::endl;
    std::cout << "c : " << c << std::endl;
    
    // Add the three integers and print the result
    int sum = a + b + c;
    std::cout << "sum: " << sum << std::endl;
    
    // Print sum2 (alternative calculation)
    int sum2 = a + b + c;
    std::cout << "sum2: " << sum2 << std::endl;
}

int main() {
    srand(time(0)); // Seed the random number generator
    
    std::cout << "Test Case 1:" << std::endl;
    generateAndPrintRandomIntegers();
    std::cout << std::endl;
    
    std::cout << "Test Case 2:" << std::endl;
    generateAndPrintRandomIntegers();
    std::cout << std::endl;
    
    std::cout << "Test Case 3:" << std::endl;
    generateAndPrintRandomIntegers();
    std::cout << std::endl;
    
    std::cout << "Test Case 4:" << std::endl;
    generateAndPrintRandomIntegers();
    std::cout << std::endl;
    
    std::cout << "Test Case 5:" << std::endl;
    generateAndPrintRandomIntegers();
    
    return 0;
}
