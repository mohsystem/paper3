#include <stdio.h>

int nb_year(int p0, double percent, int aug, int p) {
    int years = 0;
    int current_population = p0;
    double percent_decimal = percent / 100.0;
    
    while (current_population < p) {
        current_population = (int)(current_population + current_population * percent_decimal + aug);
        years++;
    }
    
    return years;
}

int main() {
    // Test Case 1
    printf("%d\n", nb_year(1500, 5, 100, 5000));
    // Test Case 2
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000));
    // Test Case 3
    printf("%d\n", nb_year(1000, 2, 50, 1200));
    // Test Case 4: Target population is already met or surpassed
    printf("%d\n", nb_year(1200, 2, 50, 1200));
    // Test Case 5: Zero percent growth
    printf("%d\n", nb_year(1000, 0, 100, 2000));
    return 0;
}