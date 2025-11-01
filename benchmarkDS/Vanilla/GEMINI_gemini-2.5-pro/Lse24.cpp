#include <iostream>
#include <random>
#include <cmath>

void generateAndCalculateDifference() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(10, 1000);

    int num1 = distrib(gen);
    int num2 = distrib(gen);

    int difference = std::abs(num1 - num2);

    std::cout << "First number: " << num1 << std::endl;
    std::cout << "Second number: " << num2 << std::endl;
    std::cout << "Difference: " << difference << std::endl;
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        generateAndCalculateDifference();
        std::cout << std::endl;
    }
    return 0;
}