
#include <iostream>
#include <random>
#include <limits>
#include <stdexcept>

// Function to generate three random integers and compute their sum
void generateAndSum() {
    // Use random_device and mt19937 for secure random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    
    // Define safe range to prevent overflow: 1001 to INT_MAX / 3
    const int max_value = std::numeric_limits<int>::max() / 3;
    std::uniform_int_distribution<int> dis(1001, max_value);
    
    // Generate three random integers larger than 1000
    int a = dis(gen);
    int b = dis(gen);
    int c = dis(gen);
    
    // Print the three integers
    std::cout << "a : " << a << std::endl;
    std::cout << "b : " << b << std::endl;
    std::cout << "c : " << c << std::endl;
    
    // Check for overflow before addition using long long
    long long sum_check = static_cast<long long>(a) + static_cast<long long>(b) + static_cast<long long>(c);
    
    if (sum_check > std::numeric_limits<int>::max() || sum_check < 0) {
        std::cerr << "Error: Sum overflow detected" << std::endl;
        return;
    }
    
    // Add the three integers and print the result
    int sum = a + b + c;
    std::cout << "sum: " << sum << std::endl;
    std::cout << "sum2: " << sum << std::endl;
}

int main() {
    // Test case 1
    std::cout << "Test case 1:" << std::endl;
    generateAndSum();
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test case 2:" << std::endl;
    generateAndSum();
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test case 3:" << std::endl;
    generateAndSum();
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test case 4:" << std::endl;
    generateAndSum();
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test case 5:" << std::endl;
    generateAndSum();
    
    return 0;
}
