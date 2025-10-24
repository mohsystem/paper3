#include <stdio.h>
#include <math.h>

int nb_year(int p0, double percent, int aug, int p) {
    if (p0 < 0 || p < 0) {
        // According to the prompt p0 and p are > 0, but as a safeguard:
        return -1; // Or handle error appropriately
    }
    int years = 0;
    double current_p = (double)p0;
    double percent_decimal = percent / 100.0;

    while (current_p < p) {
        current_p = floor(current_p + current_p * percent_decimal + aug);
        years++;
    }

    return years;
}

int main() {
    // Test Case 1
    printf("Test Case 1: nb_year(1500, 5, 100, 5000)\n");
    printf("Expected: 15\n");
    printf("Actual: %d\n\n", nb_year(1500, 5, 100, 5000));

    // Test Case 2
    printf("Test Case 2: nb_year(1500000, 2.5, 10000, 2000000)\n");
    printf("Expected: 10\n");
    printf("Actual: %d\n\n", nb_year(1500000, 2.5, 10000, 2000000));

    // Test Case 3
    printf("Test Case 3: nb_year(1000, 2, 50, 1200)\n");
    printf("Expected: 3\n");
    printf("Actual: %d\n\n", nb_year(1000, 2, 50, 1200));

    // Test Case 4: Population already at or above target
    printf("Test Case 4: nb_year(1200, 2, 50, 1200)\n");
    printf("Expected: 0\n");
    printf("Actual: %d\n\n", nb_year(1200, 2, 50, 1200));

    // Test Case 5: Zero percent growth
    printf("Test Case 5: nb_year(1000, 0, 100, 1500)\n");
    printf("Expected: 5\n");
    printf("Actual: %d\n\n", nb_year(1000, 0, 100, 1500));

    return 0;
}