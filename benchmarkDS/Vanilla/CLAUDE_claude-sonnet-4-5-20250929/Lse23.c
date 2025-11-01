
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void generateAndPrintRandomIntegers() {
    // Generate three random integers larger than 1000
    int a = rand() % 9000 + 1001; // generates numbers from 1001 to 10000
    int b = rand() % 9000 + 1001;
    int c = rand() % 9000 + 1001;
    
    // Print the three integers
    printf("a : %d\\n", a);
    printf("b : %d\\n", b);
    printf("c : %d\\n", c);
    
    // Add the three integers and print the result
    int sum = a + b + c;
    printf("sum: %d\\n", sum);
    
    // Print sum2 (alternative calculation)
    int sum2 = a + b + c;
    printf("sum2: %d\\n", sum2);
}

int main() {
    srand(time(NULL)); // Seed the random number generator
    
    printf("Test Case 1:\\n");
    generateAndPrintRandomIntegers();
    printf("\\n");
    
    printf("Test Case 2:\\n");
    generateAndPrintRandomIntegers();
    printf("\\n");
    
    printf("Test Case 3:\\n");
    generateAndPrintRandomIntegers();
    printf("\\n");
    
    printf("Test Case 4:\\n");
    generateAndPrintRandomIntegers();
    printf("\\n");
    
    printf("Test Case 5:\\n");
    generateAndPrintRandomIntegers();
    
    return 0;
}
