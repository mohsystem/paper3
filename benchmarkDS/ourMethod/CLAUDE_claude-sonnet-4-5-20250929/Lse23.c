
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdint.h>

// Function to generate three random integers and compute their sum
void generateAndSum(void) {
    int a, b, c;
    int64_t sum_check;
    int sum;
    
    // Seed random number generator with current time
    // Note: For production use, consider using a more secure random source
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    // Define safe maximum value to prevent overflow: INT_MAX / 3
    const int max_value = INT_MAX / 3;
    const int range = max_value - 1001;
    
    // Validate range to prevent modulo bias and negative values
    if (range <= 0) {
        fprintf(stderr, "Error: Invalid range\\n");
        return;
    }
    
    // Generate three random integers larger than 1000
    // Using modulo with range check to stay within safe bounds
    a = 1001 + (rand() % range);
    b = 1001 + (rand() % range);
    c = 1001 + (rand() % range);
    
    // Validate generated values are positive and in expected range
    if (a < 1001 || b < 1001 || c < 1001) {
        fprintf(stderr, "Error: Invalid generated value\\n");
        return;
    }
    
    // Print the three integers using safe printf
    printf("a : %d\\n", a);
    printf("b : %d\\n", b);
    printf("c : %d\\n", c);
    
    // Check for overflow before addition using 64-bit integer
    sum_check = (int64_t)a + (int64_t)b + (int64_t)c;
    
    if (sum_check > INT_MAX || sum_check < 0) {
        fprintf(stderr, "Error: Sum overflow detected\\n");
        return;
    }
    
    // Add the three integers and print the result
    sum = a + b + c;
    printf("sum: %d\\n", sum);
    printf("sum2: %d\\n", sum);
}

int main(void) {
    // Test case 1
    printf("Test case 1:\\n");
    generateAndSum();
    printf("\\n");
    
    // Test case 2
    printf("Test case 2:\\n");
    generateAndSum();
    printf("\\n");
    
    // Test case 3
    printf("Test case 3:\\n");
    generateAndSum();
    printf("\\n");
    
    // Test case 4
    printf("Test case 4:\\n");
    generateAndSum();
    printf("\\n");
    
    // Test case 5
    printf("Test case 5:\\n");
    generateAndSum();
    
    return 0;
}
