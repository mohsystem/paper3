#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief Returns a random sales figure between 0 and 100000, inclusive.
 * 
 * @return An integer representing the monthly sales.
 */
int getMonthlySales() {
    return rand() % 100001;
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    for (int i = 1; i <= 5; ++i) {
        printf("--- Test Case %d ---\n", i);
        
        int quarter1Sales = 0;
        int quarter2Sales = 0;
        int quarter3Sales = 0;
        int quarter4Sales = 0;

        // Call getMonthlySales 12 times, once for each month
        for (int month = 1; month <= 12; ++month) {
            int sales = getMonthlySales();
            if (month <= 3) {
                quarter1Sales += sales;
            } else if (month <= 6) {
                quarter2Sales += sales;
            } else if (month <= 9) {
                quarter3Sales += sales;
            } else {
                quarter4Sales += sales;
            }
        }

        // Print the results for each quarter
        printf("Quarter 1 Sales: %d\n", quarter1Sales);
        printf("Quarter 2 Sales: %d\n", quarter2Sales);
        printf("Quarter 3 Sales: %d\n", quarter3Sales);
        printf("Quarter 4 Sales: %d\n", quarter4Sales);
        printf("\n");
    }
    
    return 0;
}