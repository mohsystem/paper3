
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

// Function to calculate number of years needed for population to reach target
// Security: All inputs validated to prevent overflow and invalid values
int nb_year(int p0, double percent, int aug, int p) {
    // Input validation: ensure all parameters are within valid ranges
    if (p0 <= 0) {
        fprintf(stderr, "Error: Initial population must be positive\\n");
        return -1;
    }
    if (p <= 0) {
        fprintf(stderr, "Error: Target population must be positive\\n");
        return -1;
    }
    if (percent < 0.0 || percent > 100.0) {
        fprintf(stderr, "Error: Percent must be between 0 and 100\\n");
        return -1;
    }
    // Validate aug is reasonable to prevent integer overflow
    if (aug < -1000000 || aug > 1000000) {
        fprintf(stderr, "Error: Augmentation must be reasonable\\n");
        return -1;
    }
    
    // If already at or above target, return 0
    if (p0 >= p) {
        return 0;
    }
    
    // Convert percent to decimal (e.g., 2 -> 0.02)
    double rate = percent / 100.0;
    
    int years = 0;
    long long current_pop = (long long)p0; // Use long long to prevent overflow
    long long target_pop = (long long)p;
    
    // Maximum iterations to prevent infinite loop (e.g., negative growth)
    const int MAX_YEARS = 100000;
    
    // Calculate population year by year
    while (current_pop < target_pop && years < MAX_YEARS) {
        // Calculate growth: current + (current * rate) + aug
        // Use long long arithmetic to detect overflow
        long long growth = (long long)floor((double)current_pop * rate);
        
        // Check for potential overflow before adding
        if (current_pop > LLONG_MAX - growth - aug) {
            fprintf(stderr, "Error: Population calculation would overflow\\n");
            return -1;
        }
        
        current_pop = current_pop + growth + aug;
        years++;
        
        // Safety check: if population becomes negative or zero with negative aug
        if (current_pop <= 0 && aug < 0) {
            fprintf(stderr, "Error: Population decreased to zero or below\\n");
            return -1;
        }
    }
    
    // Check if loop terminated due to max iterations (indicates problem)
    if (years >= MAX_YEARS) {
        fprintf(stderr, "Error: Maximum iterations reached - check input parameters\\n");
        return -1;
    }
    
    return years;
}

int main(void) {
    int result = 0;
    
    // Test case 1: Basic example from problem description
    result = nb_year(1000, 2.0, 50, 1200);
    printf("Test 1: %d (expected: 3)\\n", result);
    
    // Test case 2: Example from problem
    result = nb_year(1500, 5.0, 100, 5000);
    printf("Test 2: %d (expected: 15)\\n", result);
    
    // Test case 3: Example from problem
    result = nb_year(1500000, 2.5, 10000, 2000000);
    printf("Test 3: %d (expected: 10)\\n", result);
    
    // Test case 4: Already at target
    result = nb_year(1000, 2.0, 50, 1000);
    printf("Test 4: %d (expected: 0)\\n", result);
    
    // Test case 5: Zero percent growth
    result = nb_year(1000, 0.0, 50, 1200);
    printf("Test 5: %d (expected: 4)\\n", result);
    
    return 0;
}
