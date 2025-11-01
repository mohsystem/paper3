
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generateRandomNumber() {
    // Generate a random number larger than 1000
    // Using rand() with time seed for pseudo-random generation
    static int initialized = 0;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = 1;
    }
    
    // Generate random number in range [1001, RAND_MAX]
    int randomNumber = 1001 + (rand() % (RAND_MAX - 1001));
    return randomNumber;
}

int main() {
    // Test case 1
    int result1 = generateRandomNumber();
    printf("Test 1 - Random number: %d (Should be > 1000)\\n", result1);
    
    // Test case 2
    int result2 = generateRandomNumber();
    printf("Test 2 - Random number: %d (Should be > 1000)\\n", result2);
    
    // Test case 3
    int result3 = generateRandomNumber();
    printf("Test 3 - Random number: %d (Should be > 1000)\\n", result3);
    
    // Test case 4
    int result4 = generateRandomNumber();
    printf("Test 4 - Random number: %d (Should be > 1000)\\n", result4);
    
    // Test case 5
    int result5 = generateRandomNumber();
    printf("Test 5 - Random number: %d (Should be > 1000)\\n", result5);
    
    return 0;
}
