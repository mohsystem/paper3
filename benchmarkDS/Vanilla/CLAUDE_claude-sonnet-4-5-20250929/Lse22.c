
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generateRandomNumber() {
    // Generate a random number larger than 1000
    return (rand() % 9000) + 1001;
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    
    // Test cases
    for (int i = 1; i <= 5; i++) {
        int randomNum = generateRandomNumber();
        printf("Test case %d: %d\\n", i, randomNum);
    }
    
    return 0;
}
