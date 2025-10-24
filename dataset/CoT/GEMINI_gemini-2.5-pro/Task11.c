#include <stdio.h>

int nb_year(int p0, double percent, int aug, int p) {
    int years = 0;
    double currentPopulation = p0;

    while (currentPopulation < p) {
        currentPopulation = currentPopulation + currentPopulation * (percent / 100.0) + aug;
        // Truncate to an integer value at the end of each year
        currentPopulation = (int)currentPopulation;
        years++;
    }
    return years;
}

int main() {
    // Test case 1
    printf("%d\n", nb_year(1500, 5, 100, 5000)); // Expected: 15

    // Test case 2
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000)); // Expected: 10

    // Test case 3
    printf("%d\n", nb_year(1000, 2, 50, 1200)); // Expected: 3
    
    // Test case 4: Target population is less than or equal to initial population
    printf("%d\n", nb_year(1200, 2, 50, 1000)); // Expected: 0

    // Test case 5: Zero percent growth
    printf("%d\n", nb_year(1000, 0, 100, 2000)); // Expected: 10

    return 0;
}