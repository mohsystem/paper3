
#include <iostream>
#include <cmath>
#include <limits>
#include <stdexcept>

// Function to calculate years needed for investment to reach desired sum
// All inputs validated to prevent undefined behavior and security issues
int calculateYears(double principal, double interest, double tax, double desired) {
    // Input validation: check for NaN and infinity to prevent undefined behavior
    if (std::isnan(principal) || std::isinf(principal) ||
        std::isnan(interest) || std::isinf(interest) ||
        std::isnan(tax) || std::isinf(tax) ||
        std::isnan(desired) || std::isinf(desired)) {
        throw std::invalid_argument("Invalid input: NaN or infinity not allowed");
    }
    
    // Validate principal is positive to prevent invalid calculations
    if (principal <= 0.0) {
        throw std::invalid_argument("Principal must be positive");
    }
    
    // Validate desired is positive
    if (desired <= 0.0) {
        throw std::invalid_argument("Desired sum must be positive");
    }
    
    // Validate interest rate is non-negative and reasonable (0 to 100%)
    if (interest < 0.0 || interest > 1.0) {
        throw std::invalid_argument("Interest rate must be between 0 and 1");
    }
    
    // Validate tax rate is non-negative and reasonable (0 to 100%)
    if (tax < 0.0 || tax > 1.0) {
        throw std::invalid_argument("Tax rate must be between 0 and 1");
    }
    
    // Handle edge case: desired equals principal
    if (desired <= principal) {
        return 0;
    }
    
    // Calculate net interest rate after tax to prevent recalculation in loop
    double netInterestRate = interest * (1.0 - tax);
    
    // Validate that investment can grow (net interest must be positive)
    if (netInterestRate <= 0.0) {
        throw std::invalid_argument("Net interest rate must be positive for growth");
    }
    
    // Initialize current principal for iteration
    double currentPrincipal = principal;
    int years = 0;
    
    // Maximum iterations to prevent infinite loop in case of numerical issues
    const int MAX_YEARS = 1000000;
    
    // Calculate years needed, checking against maximum to prevent DoS
    while (currentPrincipal < desired && years < MAX_YEARS) {
        // Calculate interest earned this year
        double yearInterest = currentPrincipal * interest;
        
        // Calculate tax on interest (not on principal)
        double taxAmount = yearInterest * tax;
        
        // Add net interest to principal
        currentPrincipal += (yearInterest - taxAmount);
        
        years++;
        
        // Additional safety check for overflow or invalid state
        if (std::isnan(currentPrincipal) || std::isinf(currentPrincipal)) {
            throw std::runtime_error("Calculation resulted in invalid value");
        }
    }
    
    // Check if maximum iterations reached without convergence
    if (years >= MAX_YEARS) {
        throw std::runtime_error("Maximum iterations reached - unable to reach desired sum");
    }
    
    return years;
}

int main() {
    try {
        // Test case 1: Example from problem description
        std::cout << "Test 1: " << calculateYears(1000.0, 0.05, 0.18, 1100.0) << std::endl;
        
        // Test case 2: Edge case where desired equals principal
        std::cout << "Test 2: " << calculateYears(1000.0, 0.05, 0.18, 1000.0) << std::endl;
        
        // Test case 3: Different values
        std::cout << "Test 3: " << calculateYears(1500.0, 0.07, 0.15, 2000.0) << std::endl;
        
        // Test case 4: Low interest rate
        std::cout << "Test 4: " << calculateYears(1000.0, 0.01, 0.10, 1050.0) << std::endl;
        
        // Test case 5: Higher tax rate
        std::cout << "Test 5: " << calculateYears(5000.0, 0.08, 0.25, 6000.0) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
