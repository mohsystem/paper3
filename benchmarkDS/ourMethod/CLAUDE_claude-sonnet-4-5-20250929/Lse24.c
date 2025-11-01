
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

typedef struct {
    int num1;
    int num2;
    int difference;
} Result;

// Initialize random seed using time
// Note: For production use, consider platform-specific secure random sources
void initRandom(void) {
    srand((unsigned int)time(NULL));
}

// Generate random number in range [min, max]
// Validates against integer overflow
int getRandomInRange(int min, int max) {
    // Validate range to prevent issues
    if (min > max) {
        fprintf(stderr, "Invalid range\\n");
        return min;
    }
    
    // Check for potential overflow in range calculation
    if (max - min > RAND_MAX) {
        fprintf(stderr, "Range too large\\n");
        return min;
    }
    
    int range = max - min + 1;
    return (rand() % range) + min;
}

Result generateAndCalculateDifference(void) {
    Result result;
    // Initialize to safe defaults
    result.num1 = 0;
    result.num2 = 0;
    result.difference = 0;
    
    // Generate random integers in range [10, 1000]
    result.num1 = getRandomInRange(10, 1000);
    result.num2 = getRandomInRange(10, 1000);
    
    // Calculate absolute difference safely
    int diff = result.num1 - result.num2;
    result.difference = (diff < 0) ? -diff : diff;
    
    return result;
}

void printResults(int num1, int num2, int difference) {
    // Validate inputs are within expected range
    if (num1 < 10 || num1 > 1000 || num2 < 10 || num2 > 1000) {
        fprintf(stderr, "Invalid input values\\n");
        return;
    }
    
    // Use constant format strings to prevent format string injection
    printf("First number: %d\\n", num1);
    printf("Second number: %d\\n", num2);
    printf("Difference: %d\\n", difference);
}

int main(void) {
    // Initialize random number generator
    initRandom();
    
    // Test case 1
    printf("Test Case 1:\\n");
    Result result1 = generateAndCalculateDifference();
    printResults(result1.num1, result1.num2, result1.difference);
    printf("\\n");
    
    // Test case 2
    printf("Test Case 2:\\n");
    Result result2 = generateAndCalculateDifference();
    printResults(result2.num1, result2.num2, result2.difference);
    printf("\\n");
    
    // Test case 3
    printf("Test Case 3:\\n");
    Result result3 = generateAndCalculateDifference();
    printResults(result3.num1, result3.num2, result3.difference);
    printf("\\n");
    
    // Test case 4
    printf("Test Case 4:\\n");
    Result result4 = generateAndCalculateDifference();
    printResults(result4.num1, result4.num2, result4.difference);
    printf("\\n");
    
    // Test case 5
    printf("Test Case 5:\\n");
    Result result5 = generateAndCalculateDifference();
    printResults(result5.num1, result5.num2, result5.difference);
    
    return 0;
}
