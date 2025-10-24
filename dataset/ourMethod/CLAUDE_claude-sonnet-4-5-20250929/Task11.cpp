
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <limits>

// Function to calculate number of years needed for population to reach target
// Security: All inputs validated to prevent overflow and invalid values
int nb_year(int p0, double percent, int aug, int p) {
    // Input validation: ensure all parameters are within valid ranges
    if (p0 <= 0) {
        throw std::invalid_argument("Initial population must be positive");
    }
    if (p <= 0) {
        throw std::invalid_argument("Target population must be positive");
    }
    if (percent < 0.0 || percent > 100.0) {
        throw std::invalid_argument("Percent must be between 0 and 100");
    }
    // Validate aug is reasonable to prevent integer overflow
    if (aug < -1000000 || aug > 1000000) {
        throw std::invalid_argument("Augmentation must be reasonable");
    }
    
    // If already at or above target, return 0
    if (p0 >= p) {
        return 0;
    }
    
    // Convert percent to decimal (e.g., 2 -> 0.02)
    double rate = percent / 100.0;
    
    int years = 0;
    long long current_pop = static_cast<long long>(p0); // Use long long to prevent overflow
    long long target_pop = static_cast<long long>(p);
    
    // Maximum iterations to prevent infinite loop (e.g., negative growth)
    const int MAX_YEARS = 100000;
    
    // Calculate population year by year
    while (current_pop < target_pop && years < MAX_YEARS) {
        // Calculate growth: current + (current * rate) + aug
        // Use long long arithmetic to detect overflow
        long long growth = static_cast<long long>(std::floor(current_pop * rate));
        
        // Check for potential overflow before adding
        if (current_pop > std::numeric_limits<long long>::max() - growth - aug) {
            throw std::overflow_error("Population calculation would overflow");
        }
        
        current_pop = current_pop + growth + aug;
        years++;
        
        // Safety check: if population becomes negative or zero with negative aug
        if (current_pop <= 0 && aug < 0) {
            throw std::runtime_error("Population decreased to zero or below");
        }
    }
    
    // Check if loop terminated due to max iterations (indicates problem)
    if (years >= MAX_YEARS) {
        throw std::runtime_error("Maximum iterations reached - check input parameters");
    }
    
    return years;
}

int main() {
    try {
        // Test case 1: Basic example from problem description
        std::cout << "Test 1: " << nb_year(1000, 2, 50, 1200) << " (expected: 3)" << std::endl;
        
        // Test case 2: Example from problem
        std::cout << "Test 2: " << nb_year(1500, 5, 100, 5000) << " (expected: 15)" << std::endl;
        
        // Test case 3: Example from problem
        std::cout << "Test 3: " << nb_year(1500000, 2.5, 10000, 2000000) << " (expected: 10)" << std::endl;
        
        // Test case 4: Already at target
        std::cout << "Test 4: " << nb_year(1000, 2, 50, 1000) << " (expected: 0)" << std::endl;
        
        // Test case 5: Zero percent growth
        std::cout << "Test 5: " << nb_year(1000, 0, 50, 1200) << " (expected: 4)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
