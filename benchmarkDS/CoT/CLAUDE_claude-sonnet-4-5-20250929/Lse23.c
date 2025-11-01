
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

void generateAndSum() {
    // Generate three random integers larger than 1000
    int a = (rand() % (INT_MAX - 1001)) + 1001;
    int b = (rand() % (INT_MAX - 1001)) + 1001;
    int c = (rand() % (INT_MAX - 1001)) + 1001;
    
    // Print the three integers
    printf("a : %d\\n", a);
    printf("b : %d\\n", b);
    printf("c : %d\\n", c);
    
    // Add the three integers and print the result
    long long sum = (long long)a + (long long)b + (long long)c;
    long long sum2 = (long long)a + (long long)b + (long long)c;
    
    printf("sum: %lld\\n", sum);
    printf("sum2: %lld\\n", sum2);
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1:\\n");
    generateAndSum();
    printf("\\nTest Case 2:\\n");
    generateAndSum();
    printf("\\nTest Case 3:\\n");
    generateAndSum();
    printf("\\nTest Case 4:\\n");
    generateAndSum();
    printf("\\nTest Case 5:\\n");
    generateAndSum();
    
    return 0;
}
