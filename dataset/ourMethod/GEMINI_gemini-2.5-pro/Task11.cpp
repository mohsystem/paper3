#include <iostream>
#include <cmath>

int nb_year(int p0, double percent, int aug, int p) {
    if (p0 < 0 || p < 0) {
        // According to the prompt p0 and p are > 0, but as a safeguard:
        return -1; // Or handle error appropriately
    }
    int years = 0;
    double current_p = static_cast<double>(p0);
    double percent_decimal = percent / 100.0;

    while (current_p < p) {
        current_p = floor(current_p + current_p * percent_decimal + aug);
        years++;
    }

    return years;
}

int main() {
    // Test Case 1
    std::cout << "Test Case 1: nb_year(1500, 5, 100, 5000)" << std::endl;
    std::cout << "Expected: 15" << std::endl;
    std::cout << "Actual: " << nb_year(1500, 5, 100, 5000) << std::endl;
    std::cout << std::endl;

    // Test Case 2
    std::cout << "Test Case 2: nb_year(1500000, 2.5, 10000, 2000000)" << std::endl;
    std::cout << "Expected: 10" << std::endl;
    std::cout << "Actual: " << nb_year(1500000, 2.5, 10000, 2000000) << std::endl;
    std::cout << std::endl;

    // Test Case 3
    std::cout << "Test Case 3: nb_year(1000, 2, 50, 1200)" << std::endl;
    std::cout << "Expected: 3" << std::endl;
    std::cout << "Actual: " << nb_year(1000, 2, 50, 1200) << std::endl;
    std::cout << std::endl;

    // Test Case 4: Population already at or above target
    std::cout << "Test Case 4: nb_year(1200, 2, 50, 1200)" << std::endl;
    std::cout << "Expected: 0" << std::endl;
    std::cout << "Actual: " << nb_year(1200, 2, 50, 1200) << std::endl;
    std::cout << std::endl;

    // Test Case 5: Zero percent growth
    std::cout << "Test Case 5: nb_year(1000, 0, 100, 1500)" << std::endl;
    std::cout << "Expected: 5" << std::endl;
    std::cout << "Actual: " << nb_year(1000, 0, 100, 1500) << std::endl;
    std::cout << std::endl;

    return 0;
}