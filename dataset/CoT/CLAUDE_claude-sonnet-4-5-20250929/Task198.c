
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Simulated rand7() API
int rand7() {
    return rand() % 7 + 1;
}

// rand10() implementation using rejection sampling
int rand10() {
    int num;
    while (1) {
        // Generate number in range [0, 48]
        num = (rand7() - 1) * 7 + (rand7() - 1);
        
        // Only accept numbers in range [0, 39] for uniform distribution
        if (num < 40) {
            return (num % 10) + 1;
        }
        // Reject and retry if num >= 40
    }
}

// Test function
void testRand10(int n, int* results) {
    for (int i = 0; i < n; i++) {
        results[i] = rand10();
    }
}

int main() {
    srand(time(NULL));
    
    // Test case 1
    int test1[1];
    testRand10(1, test1);
    printf("Test 1: [");
    for (int i = 0; i < 1; i++) {
        printf("%d%s", test1[i], i < 0 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 2
    int test2[2];
    testRand10(2, test2);
    printf("Test 2: [");
    for (int i = 0; i < 2; i++) {
        printf("%d%s", test2[i], i < 1 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 3
    int test3[3];
    testRand10(3, test3);
    printf("Test 3: [");
    for (int i = 0; i < 3; i++) {
        printf("%d%s", test3[i], i < 2 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 4
    int test4[5];
    testRand10(5, test4);
    printf("Test 4: [");
    for (int i = 0; i < 5; i++) {
        printf("%d%s", test4[i], i < 4 ? "," : "");
    }
    printf("]\\n");
    
    // Test case 5
    int test5[10];
    testRand10(10, test5);
    printf("Test 5: [");
    for (int i = 0; i < 10; i++) {
        printf("%d%s", test5[i], i < 9 ? "," : "");
    }
    printf("]\\n");
    
    return 0;
}
