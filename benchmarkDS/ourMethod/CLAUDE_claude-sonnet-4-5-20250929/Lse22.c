
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

/**
 * Generates a random number larger than 1000.
 * Uses time-seeded rand() with range validation.
 * Note: For cryptographic purposes, use platform-specific CSPRNG APIs.
 * 
 * @return A random integer greater than 1000
 */
int generateRandomNumber(void) {
    static int seeded = 0;
    
    // Seed only once to prevent predictable sequences
    if (!seeded) {
        // Use time as seed - not cryptographically secure but acceptable
        // for non-security-critical random number generation
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    // Generate random number avoiding overflow
    // RAND_MAX is guaranteed to be at least 32767
    // Scale to larger range and add 1001 to ensure > 1000
    int random_value = rand();
    
    // Ensure result is greater than 1000
    // Use modulo to limit range and prevent overflow when adding
    int result = (random_value % (INT_MAX - 1001)) + 1001;
    
    return result;
}

int main(void) {
    // Test case 1
    int result1 = generateRandomNumber();
    printf("Test 1 - Random number: %d (should be > 1000)\\n", result1);
    
    // Test case 2
    int result2 = generateRandomNumber();
    printf("Test 2 - Random number: %d (should be > 1000)\\n", result2);
    
    // Test case 3
    int result3 = generateRandomNumber();
    printf("Test 3 - Random number: %d (should be > 1000)\\n", result3);
    
    // Test case 4
    int result4 = generateRandomNumber();
    printf("Test 4 - Random number: %d (should be > 1000)\\n", result4);
    
    // Test case 5
    int result5 = generateRandomNumber();
    printf("Test 5 - Random number: %d (should be > 1000)\\n", result5);
    
    return 0;
}
