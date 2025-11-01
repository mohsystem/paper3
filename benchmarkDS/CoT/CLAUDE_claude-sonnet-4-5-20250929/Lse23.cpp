
#include <iostream>
#include <random>

void generateAndSum() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(1001, INT_MAX);
    
    // Generate three random integers larger than 1000
    int a = dis(gen);
    int b = dis(gen);
    int c = dis(gen);
    
    // Print the three integers
    std::cout << "a : " << a << std::endl;
    std::cout << "b : " << b << std::endl;
    std::cout << "c : " << c << std::endl;
    
    // Add the three integers and print the result
    long long sum = (long long)a + (long long)b + (long long)c;
    long long sum2 = (long long)a + (long long)b + (long long)c;
    
    std::cout << "sum: " << sum << std::endl;
    std::cout << "sum2: " << sum2 << std::endl;
}

int main() {
    std::cout << "Test Case 1:" << std::endl;
    generateAndSum();
    std::cout << "\\nTest Case 2:" << std::endl;
    generateAndSum();
    std::cout << "\\nTest Case 3:" << std::endl;
    generateAndSum();
    std::cout << "\\nTest Case 4:" << std::endl;
    generateAndSum();
    std::cout << "\\nTest Case 5:" << std::endl;
    generateAndSum();
    
    return 0;
}
