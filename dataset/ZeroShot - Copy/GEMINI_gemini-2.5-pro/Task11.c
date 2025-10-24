#include <stdio.h>

/**
 * Calculates the number of years required for a population to reach a target.
 *
 * @param p0      The initial population.
 * @param percent The annual percentage increase.
 * @param aug     The number of inhabitants coming or leaving each year.
 * @param p       The target population to surpass.
 * @return The number of full years needed.
 */
int nb_year(int p0, double percent, int aug, int p) {
    int years = 0;
    int current_population = p0;
    double percent_decimal = percent / 100.0;

    while (current_population < p) {
        // Calculate the population for the next year and truncate to the nearest whole number.
        current_population = (int)(current_population + current_population * percent_decimal + aug);
        years++;
    }
    return years;
}

int main() {
    // Test Case 1: From example
    printf("%d\n", nb_year(1500, 5, 100, 5000));
    // Test Case 2: From example
    printf("%d\n", nb_year(1500000, 2.5, 10000, 2000000));
    // Test Case 3: Initial prompt example
    printf("%d\n", nb_year(1000, 2, 50, 1200));
    // Test Case 4: Initial population is already at or above target
    printf("%d\n", nb_year(1200, 2, 50, 1200));
    // Test Case 5: Small growth
    printf("%d\n", nb_year(1000, 0.5, 2, 1010));

    return 0;
}