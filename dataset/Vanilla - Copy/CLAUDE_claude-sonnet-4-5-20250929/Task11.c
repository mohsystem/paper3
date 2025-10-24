
#include <stdio.h>

int nbYear(int p0, double percent, int aug, int p) {
    int years = 0;
    int currentPopulation = p0;
    double percentDecimal = percent / 100.0;
    
    while (currentPopulation < p) {
        currentPopulation = (int)(currentPopulation + currentPopulation * percentDecimal + aug);
        years++;
    }
    
    return years;
}

int main() {
    // Test cases
    printf("Test 1: %d (Expected: 3)\\n", nbYear(1000, 2, 50, 1200));
    printf("Test 2: %d (Expected: 15)\\n", nbYear(1500, 5, 100, 5000));
    printf("Test 3: %d (Expected: 10)\\n", nbYear(1500000, 2.5, 10000, 2000000));
    printf("Test 4: %d (Expected: 10)\\n", nbYear(1000, 2, 0, 1200));
    printf("Test 5: %d (Expected: 10)\\n", nbYear(1000, 0, 50, 1500));
    
    return 0;
}
