
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

void generateAndCalculateDifference() {
    // Generate random integers between 10 and 1000 (inclusive)
    int num1 = (rand() % 991) + 10;
    int num2 = (rand() % 991) + 10;
    
    // Calculate the absolute difference
    int difference = abs(num1 - num2);
    
    // Print the results
    printf("First number: %d\\n", num1);
    printf("Second number: %d\\n", num2);
    printf("Difference: %d\\n", difference);
}

int main() {
    // Seed the random number generator with current time
    srand((unsigned int)time(NULL));
    
    printf("Test Case 1:\\n");
    generateAndCalculateDifference();
    printf("\\n");
    
    printf("Test Case 2:\\n");
    generateAndCalculateDifference();
    printf("\\n");
    
    printf("Test Case 3:\\n");
    generateAndCalculateDifference();
    printf("\\n");
    
    printf("Test Case 4:\\n");
    generateAndCalculateDifference();
    printf("\\n");
    
    printf("Test Case 5:\\n");
    generateAndCalculateDifference();
    
    return 0;
}
