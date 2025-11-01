
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define MAX_SALES 100000

int getMonthlySales() {
    return rand() % (MAX_SALES + 1);
}

void calculateQuarterlySales() {
    int64_t q1 = 0, q2 = 0, q3 = 0, q4 = 0;
    
    // Quarter 1 (Months 1-3)
    for (int i = 0; i < 3; i++) {
        q1 += getMonthlySales();
    }
    
    // Quarter 2 (Months 4-6)
    for (int i = 0; i < 3; i++) {
        q2 += getMonthlySales();
    }
    
    // Quarter 3 (Months 7-9)
    for (int i = 0; i < 3; i++) {
        q3 += getMonthlySales();
    }
    
    // Quarter 4 (Months 10-12)
    for (int i = 0; i < 3; i++) {
        q4 += getMonthlySales();
    }
    
    printf("Quarterly Sales Report:\\n");
    printf("Q1 Sales: $%lld\\n", (long long)q1);
    printf("Q2 Sales: $%lld\\n", (long long)q2);
    printf("Q3 Sales: $%lld\\n", (long long)q3);
    printf("Q4 Sales: $%lld\\n", (long long)q4);
    printf("Total Annual Sales: $%lld\\n", (long long)(q1 + q2 + q3 + q4));
    printf("\\n");
}

int main() {
    srand((unsigned int)time(NULL));
    
    printf("Running 5 test cases:\\n\\n");
    for (int i = 1; i <= 5; i++) {
        printf("Test Case %d:\\n", i);
        calculateQuarterlySales();
    }
    return 0;
}
