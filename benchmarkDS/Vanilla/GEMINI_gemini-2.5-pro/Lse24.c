#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateAndCalculateDifference() {
    int num1 = (rand() % (1000 - 10 + 1)) + 10;
    int num2 = (rand() % (1000 - 10 + 1)) + 10;
    
    int difference = abs(num1 - num2);
    
    printf("First number: %d\n", num1);
    printf("Second number: %d\n", num2);
    printf("Difference: %d\n", difference);
}

int main() {
    srand(time(NULL));

    for (int i = 1; i <= 5; i++) {
        printf("--- Test Case %d ---\n", i);
        generateAndCalculateDifference();
        printf("\n");
    }

    return 0;
}