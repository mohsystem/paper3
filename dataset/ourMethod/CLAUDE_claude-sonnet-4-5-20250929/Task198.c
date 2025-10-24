
#include <stdio.h>
#include <stdlib.h>

/* Simulated rand7() API for testing - generates uniform random in [1,7] */
int rand7(void) {
    /* In production, this would be the provided API */
    /* For testing, using a simple implementation */
    static unsigned int seed = 123456789U;
    seed = (1103515245U * seed + 12345U) & 0x7fffffffU;
    return (int)((seed % 7U) + 1U);
}

/* Generate uniform random integer in [1, 10] using only rand7() */
/* Security: Uses rejection sampling to ensure uniform distribution */
/* Avoids modulo bias by rejecting values outside uniform range */
int rand10(void) {
    int result;
    
    /* Generate uniform random in range [1, 49] using two rand7() calls */
    /* This creates a 7x7 grid of equally probable outcomes */
    while (1) {
        int row = rand7();  /* [1, 7] */
        int col = rand7();  /* [1, 7] */
        
        /* Bounds check: ensure row and col are in valid range */
        if (row < 1 || row > 7 || col < 1 || col > 7) {
            continue;  /* Defensive: should never happen with valid rand7() */
        }
        
        /* Map to [1, 49] - no overflow risk as max is 6*7+7=49 */
        result = (row - 1) * 7 + col;
        
        /* Accept only [1, 40] to ensure uniform distribution over [1, 10] */
        /* 40 is largest multiple of 10 within [1, 49] */
        /* This avoids modulo bias */
        if (result <= 40) {
            /* Map [1, 40] uniformly to [1, 10] */
            /* (result - 1) % 10 gives [0, 9], then add 1 for [1, 10] */
            return ((result - 1) % 10) + 1;
        }
        
        /* Rejection: result in [41, 49], retry to maintain uniformity */
        /* Expected iterations: 49/40 ≈ 1.225 */
    }
}

/* Test harness */
int main(void) {
    int i;
    
    /* Test case 1: n = 1 */
    printf("Test 1 (n=1): [%d]\\n", rand10());
    
    /* Test case 2: n = 2 */
    printf("Test 2 (n=2): [%d,%d]\\n", rand10(), rand10());
    
    /* Test case 3: n = 3 */
    printf("Test 3 (n=3): [%d,%d,%d]\\n", rand10(), rand10(), rand10());
    
    /* Test case 4: n = 5 - verify distribution */
    printf("Test 4 (n=5): [");
    for (i = 0; i < 5; i++) {
        if (i > 0) printf(",");
        printf("%d", rand10());
    }
    printf("]\\n");
    
    /* Test case 5: n = 10 - larger sample */
    printf("Test 5 (n=10): [");
    for (i = 0; i < 10; i++) {
        if (i > 0) printf(",");
        printf("%d", rand10());
    }
    printf("]\\n");
    
    return 0;
}
