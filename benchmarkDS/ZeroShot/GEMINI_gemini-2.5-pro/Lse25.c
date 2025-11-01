#include <stdio.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h>   // For time()

/**
 * @brief Generates a random sales figure for a month.
 * Note: rand() is not cryptographically secure. For security needs, a
 * different library would be required.
 * @return An integer between 0 and 100000, inclusive.
 */
int getMonthlySales() {
    // The result of rand() is modulo 100001 to get a number in the range [0, 100000].
    // This can introduce a slight bias, but is acceptable for simple cases.
    return rand() % 100001;
}

/**
 * @brief Calculates and prints the total sales for each of the four quarters.
 */
void calculateAndPrintQuarterlySales() {
    // Use long to prevent potential overflow, although int is likely sufficient.
    long quarterly_sales[4] = {0, 0, 0, 0};

    for (int month = 1; month <= 12; month++) {
        int sale = getMonthlySales();
        // Determine the quarter index (0-3)
        int quarter_index = (month - 1) / 3;
        quarterly_sales[quarter_index] += sale;
    }

    printf("Quarter 1 Sales: $%ld\n", quarterly_sales[0]);
    printf("Quarter 2 Sales: $%ld\n", quarterly_sales[1]);
    printf("Quarter 3 Sales: $%ld\n", quarterly_sales[2]);
    printf("Quarter 4 Sales: $%ld\n", quarterly_sales[3]);
}

int main() {
    // Seed the random number generator once at the beginning of the program.
    // Using the current time ensures a different sequence of random numbers on each run.
    srand((unsigned int)time(NULL));

    for (int i = 1; i <= 5; i++) {
        printf("--- Test Case %d ---\n", i);
        calculateAndPrintQuarterlySales();
        printf("\n");
    }

    return 0;
}