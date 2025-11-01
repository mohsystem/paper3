
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateAndCalculate() {
    // Generate first random integer between 10 and 1000
    int num1 = rand() % 991 + 10;
    
    // Generate second random integer between 10 and 1000
    int num2 = rand() % 991 + 10;
    
    // Calculate the difference between the two integers
    int difference = num1 - num2;
    
    // Print the results
    printf("First number: %d\\n", num1);
    printf("Second number: %d\\n", num2);
    printf("Difference: %d\\n", difference);
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1:\\n");
    generateAndCalculate();
    printf("\\n");
    
    printf("Test Case 2:\\n");
    generateAndCalculate();
    printf("\\n");
    
    printf("Test Case 3:\\n");
    generateAndCalculate();
    printf("\\n");
    
    printf("Test Case 4:\\n");
    generateAndCalculate();
    printf("\\n");
    
    printf("Test Case 5:\\n");
    generateAndCalculate();
    
    return 0;
}
