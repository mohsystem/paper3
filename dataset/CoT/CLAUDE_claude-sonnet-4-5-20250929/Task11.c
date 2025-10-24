
#include <stdio.h>
#include <stdlib.h>

int nb_year(int p0, double percent, int aug, int p) {
    // Input validation to prevent security issues
    if (p0 <= 0 || p <= 0 || percent < 0) {
        fprintf(stderr, "Error: Invalid input parameters\\n");
        exit(1);
    }
    
    int years = 0;
    long long currentPopulation = p0;
    
    // Convert percent to decimal (e.g., 2 -> 0.02)
    double percentDecimal = percent / 100.0;
    
    // Calculate years needed with overflow protection
    const int MAX_ITERATIONS = 10000;
    while (currentPopulation < p) {
        // Prevent infinite loop
        if (years >= MAX_ITERATIONS) {
            fprintf(stderr, "Error: Calculation exceeded maximum iterations\\n");
            exit(1);
        }
        
        // Calculate new population with integer conversion (floor)
        long long newPopulation = currentPopulation + 
                                 (long long)(currentPopulation * percentDecimal) + 
                                 aug;
        
        // Check for overflow
        if (newPopulation < 0 || newPopulation > 1000000000000000LL) {
            fprintf(stderr, "Error: Population overflow occurred\\n");
            exit(1);
        }
        
        currentPopulation = newPopulation;
        years++;
    }
    
    return years;
}

int main() {
    // Test case 1
    printf("Test 1: %d (Expected: 3)\\n", nb_year(1000, 2.0, 50, 1200));
    
    // Test case 2
    printf("Test 2: %d (Expected: 15)\\n", nb_year(1500, 5.0, 100, 5000));
    
    // Test case 3
    printf("Test 3: %d (Expected: 10)\\n", nb_year(1500000, 2.5, 10000, 2000000));
    
    // Test case 4
    printf("Test 4: %d (Expected: 4)\\n", nb_year(1000, 0.0, 50, 1200));
    
    // Test case 5
    printf("Test 5: %d (Expected: 14)\\n", nb_year(1500, 10.0, 200, 10000));
    
    return 0;
}
