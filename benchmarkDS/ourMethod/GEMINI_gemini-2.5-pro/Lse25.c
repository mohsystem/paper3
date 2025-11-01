#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SALES 100000
#define MONTHS_IN_YEAR 12

/**
 * Generates a random monthly sales figure.
 * Note: rand() is not cryptographically secure and may have poor
 * statistical properties. It is sufficient for this simple simulation.
 * @return A random integer between 0 and 100000 (inclusive).
 */
int getMonthlySales(void) {
    // Using modulo might introduce a slight bias if RAND_MAX is not a multiple of
    // (MAX_SALES + 1), but for this range it is generally negligible.
    return rand() % (MAX_SALES + 1);
}

/**
 * Runs a single yearly simulation: gets 12 monthly sales,
 * calculates quarterly totals, and prints the results.
 */
void runSimulation(void) {
    int monthlySales[MONTHS_IN_YEAR];

    // Get sales for each month
    for (int i = 0; i < MONTHS_IN_YEAR; i++) {
        monthlySales[i] = getMonthlySales();
    }

    // Calculate and print quarterly sales.
    // Use long to prevent potential integer overflow on sum.
    long quarter1Sales = 0;
    long quarter2Sales = 0;
    long quarter3Sales = 0;
    long quarter4Sales = 0;

    for (int i = 0; i < 3; i++) {
        quarter1Sales += monthlySales[i];
    }
    for (int i = 3; i < 6; i++) {
        quarter2Sales += monthlySales[i];
    }
    for (int i = 6; i < 9; i++) {
        quarter3Sales += monthlySales[i];
    }
    for (int i = 9; i < 12; i++) {
        quarter4Sales += monthlySales[i];
    }

    printf("Quarter 1 Sales: %ld\n", quarter1Sales);
    printf("Quarter 2 Sales: %ld\n", quarter2Sales);
    printf("Quarter 3 Sales: %ld\n", quarter3Sales);
    printf("Quarter 4 Sales: %ld\n", quarter4Sales);
}

/**
 * Main function to run the simulation 5 times as test cases.
 */
int main(void) {
    // Seed the random number generator once at the start of the program
    srand((unsigned int)time(NULL));

    for (int i = 1; i <= 5; i++) {
        printf("--- Test Case %d ---\n", i);
        runSimulation();
        if (i < 5) {
            printf("\n");
        }
    }

    return 0;
}