#include <iostream>

int nbYear(int p0, double percent, int aug, int p) {
    int years = 0;
    double currentPopulation = p0;
    double percentDecimal = percent / 100.0;

    while (currentPopulation < p) {
        currentPopulation = currentPopulation + currentPopulation * percentDecimal + aug;
        currentPopulation = (int)currentPopulation; // Truncate
        years++;
    }
    return years;
}

int main() {
    // Test Case 1
    std::cout << nbYear(1500, 5, 100, 5000) << std::endl;
    // Test Case 2
    std::cout << nbYear(1500000, 2.5, 10000, 2000000) << std::endl;
    // Test Case 3
    std::cout << nbYear(1000, 2, 50, 1200) << std::endl;
    // Test Case 4: Target population is already met or surpassed
    std::cout << nbYear(1200, 2, 50, 1200) << std::endl;
    // Test Case 5: Zero percent growth
    std::cout << nbYear(1000, 0, 100, 2000) << std::endl;
    return 0;
}