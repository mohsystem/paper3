
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Simulated rand7() function for testing
int rand7() {
    return rand() % 7 + 1;
}

int rand10() {
    int result;
    do {
        // Generate a number in range [1, 49]
        int row = rand7();
        int col = rand7();
        result = (row - 1) * 7 + col;
    } while (result > 40); // Reject numbers > 40 to maintain uniform distribution
    
    // Map [1, 40] to [1, 10]
    return (result - 1) % 10 + 1;
}

void testRand10(int n, int* results) {
    for (int i = 0; i < n; i++) {
        results[i] = rand10();
    }
}

int main() {
    srand(time(NULL));
    
    // Test case 1
    int result1[1];
    testRand10(1, result1);
    printf("Test 1 (n=1): [");
    for (int i = 0; i < 1; i++) {
        printf("%d%s", result1[i], i < 0 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 2
    int result2[2];
    testRand10(2, result2);
    printf("Test 2 (n=2): [");
    for (int i = 0; i < 2; i++) {
        printf("%d%s", result2[i], i < 1 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 3
    int result3[3];
    testRand10(3, result3);
    printf("Test 3 (n=3): [");
    for (int i = 0; i < 3; i++) {
        printf("%d%s", result3[i], i < 2 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 4
    int result4[5];
    testRand10(5, result4);
    printf("Test 4 (n=5): [");
    for (int i = 0; i < 5; i++) {
        printf("%d%s", result4[i], i < 4 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 5
    int result5[10];
    testRand10(10, result5);
    printf("Test 5 (n=10): [");
    for (int i = 0; i < 10; i++) {
        printf("%d%s", result5[i], i < 9 ? "," : "");
    }
    printf("]\\n");
    
    return 0;
}
