
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Simulating rand7() API
int rand7() {
    return rand() % 7 + 1;
}

int rand10() {
    while (1) {
        // Generate a number in range [1, 49]
        int row = rand7();
        int col = rand7();
        int num = (row - 1) * 7 + col;
        
        // Only use numbers 1-40 to ensure uniformity
        if (num <= 40) {
            return (num - 1) % 10 + 1;
        }
    }
}

int main() {
    srand(time(NULL));
    
    // Test case 1: n = 1
    printf("Test 1 (n=1): [");
    for (int i = 0; i < 1; i++) {
        printf("%d", rand10());
        if (i < 0) printf(",");
    }
    printf("]\\n");
    
    // Test case 2: n = 2
    printf("Test 2 (n=2): [");
    for (int i = 0; i < 2; i++) {
        printf("%d", rand10());
        if (i < 1) printf(",");
    }
    printf("]\\n");
    
    // Test case 3: n = 3
    printf("Test 3 (n=3): [");
    for (int i = 0; i < 3; i++) {
        printf("%d", rand10());
        if (i < 2) printf(",");
    }
    printf("]\\n");
    
    // Test case 4: n = 5
    printf("Test 4 (n=5): [");
    for (int i = 0; i < 5; i++) {
        printf("%d", rand10());
        if (i < 4) printf(",");
    }
    printf("]\\n");
    
    // Test case 5: n = 10
    printf("Test 5 (n=10): [");
    for (int i = 0; i < 10; i++) {
        printf("%d", rand10());
        if (i < 9) printf(",");
    }
    printf("]\\n");
    
    return 0;
}
