
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generateRandomNumber() {
    // Generate a random number between 1001 and 100000
    int randomNumber = (rand() % 99000) + 1001;
    return randomNumber;
}

void printRandomNumber(int number) {
    if (number > 1000) {
        printf("Generated random number: %d\\n", number);
    } else {
        printf("Error: Number must be larger than 1000\\n");
    }
}

int main() {
    // Seed the random number generator
    srand((unsigned int)time(NULL));
    
    // Test cases
    for (int i = 0; i < 5; i++) {
        int randomNum = generateRandomNumber();
        printRandomNumber(randomNum);
    }
    return 0;
}
