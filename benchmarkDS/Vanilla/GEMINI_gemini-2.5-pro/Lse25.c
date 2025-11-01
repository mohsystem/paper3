#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Returns a random number between 0 and 100000.
 * Requires srand() to be called once in the main program.
 * @return int A random integer representing monthly sales.
 */
int getMonthlySales() {
    return rand() % 100001; // Generates a random number from 0 to 100000
}

/**
 * @brief Simulates a year of sales, calculates the total for each quarter,
 * and prints the results.
 */
void calculateAndPrintQuarterlySales() {
    int quarter1Sales = 0;
    int quarter2Sales = 0;
    int quarter3Sales = 0;
    int quarter4Sales = 0;

    // Loop through 12 months of the year
    for (int month = 1; month <= 12; month++) {
        int monthlySale = getMonthlySales();
        if (month <= 3) {
            quarter1Sales += monthlySale;
        } else if (month <= 6) {
            quarter2Sales += monthlySale;
        } else if (month <= 9) {
            quarter3Sales += monthlySale;
        } else {
            quarter4Sales += monthlySale;
        }
    }

    // Print the results for each quarter
    printf("Quarter 1 Sales: %d\n", quarter1Sales);
    printf("Quarter 2 Sales: %d\n", quarter2Sales);
    printf("Quarter 3 Sales: %d\n", quarter3Sales);
    printf("Quarter 4 Sales: %d\n", quarter4Sales);
    printf("--------------------\n");
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    // Run 5 test cases
    for (int i = 0; i < 5; i++) {
        printf("Test Case %d\n", i + 1);
        calculateAndPrintQuarterlySales();
    }

    return 0;
}