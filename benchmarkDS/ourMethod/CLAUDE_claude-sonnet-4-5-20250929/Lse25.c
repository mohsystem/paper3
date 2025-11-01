
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

/**
 * Returns a random sales value between 0 and 100000 (inclusive).
 * Uses rand() with proper seeding for random number generation.
 * Note: For production code, consider using arc4random or /dev/urandom
 * 
 * @return random sales value in range [0, 100000]
 */
int getMonthlySales(void) {
    // rand() returns [0, RAND_MAX], we scale to [0, 100000]
    // Using modulo with proper range checking
    if (RAND_MAX < 100000) {
        fprintf(stderr, "Error: RAND_MAX too small\\n");
        return 0;
    }
    return rand() % 100001;
}

/**
 * Calculates quarterly sales by calling getMonthlySales 12 times
 * and aggregating results into 4 quarters.
 */
void calculateQuarterlySales(void) {
    // Initialize quarterly sales - explicit initialization to zero
    int q1 = 0;
    int q2 = 0;
    int q3 = 0;
    int q4 = 0;
    int sales = 0;
    int i = 0;
    
    // Quarter 1: January, February, March (months 0-2)
    for (i = 0; i < 3; i++) {
        sales = getMonthlySales();
        // Check for integer overflow before addition
        if (q1 > INT_MAX - sales) {
            fprintf(stderr, "Error: Integer overflow detected in Q1\\n");
            return;
        }
        q1 += sales;
    }
    
    // Quarter 2: April, May, June (months 3-5)
    for (i = 0; i < 3; i++) {
        sales = getMonthlySales();
        if (q2 > INT_MAX - sales) {
            fprintf(stderr, "Error: Integer overflow detected in Q2\\n");
            return;
        }
        q2 += sales;
    }
    
    // Quarter 3: July, August, September (months 6-8)
    for (i = 0; i < 3; i++) {
        sales = getMonthlySales();
        if (q3 > INT_MAX - sales) {
            fprintf(stderr, "Error: Integer overflow detected in Q3\\n");
            return;
        }
        q3 += sales;
    }
    
    // Quarter 4: October, November, December (months 9-11)
    for (i = 0; i < 3; i++) {
        sales = getMonthlySales();
        if (q4 > INT_MAX - sales) {
            fprintf(stderr, "Error: Integer overflow detected in Q4\\n");
            return;
        }
        q4 += sales;
    }
    
    // Print results using constant format strings for security
    printf("Q1 Sales: %d\\n", q1);
    printf("Q2 Sales: %d\\n", q2);
    printf("Q3 Sales: %d\\n", q3);
    printf("Q4 Sales: %d\\n", q4);
}

int main(void) {
    // Seed random number generator once at startup
    // Using time for seed (for better randomness, use /dev/urandom in production)
    srand((unsigned int)time(NULL));
    
    // Test case 1
    printf("Test Case 1:\\n");
    calculateQuarterlySales();
    printf("\\n");
    
    // Test case 2
    printf("Test Case 2:\\n");
    calculateQuarterlySales();
    printf("\\n");
    
    // Test case 3
    printf("Test Case 3:\\n");
    calculateQuarterlySales();
    printf("\\n");
    
    // Test case 4
    printf("Test Case 4:\\n");
    calculateQuarterlySales();
    printf("\\n");
    
    // Test case 5
    printf("Test Case 5:\\n");
    calculateQuarterlySales();
    
    return 0;
}
