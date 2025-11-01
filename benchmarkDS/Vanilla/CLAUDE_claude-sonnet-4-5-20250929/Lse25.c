
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getMonthlySales() {
    return rand() % 100001;
}

void calculateQuarterlySales() {
    int monthlySales[12];
    
    // Get monthly sales for all 12 months
    for (int i = 0; i < 12; i++) {
        monthlySales[i] = getMonthlySales();
    }
    
    // Calculate quarterly sales
    int q1 = monthlySales[0] + monthlySales[1] + monthlySales[2];
    int q2 = monthlySales[3] + monthlySales[4] + monthlySales[5];
    int q3 = monthlySales[6] + monthlySales[7] + monthlySales[8];
    int q4 = monthlySales[9] + monthlySales[10] + monthlySales[11];
    
    // Print results
    printf("Quarter 1 Sales: $%d\\n", q1);
    printf("Quarter 2 Sales: $%d\\n", q2);
    printf("Quarter 3 Sales: $%d\\n", q3);
    printf("Quarter 4 Sales: $%d\\n", q4);
    printf("Total Annual Sales: $%d\\n", q1 + q2 + q3 + q4);
    printf("\\n");
}

int main() {
    srand(time(NULL));
    
    printf("Test Case 1:\\n");
    calculateQuarterlySales();
    
    printf("Test Case 2:\\n");
    calculateQuarterlySales();
    
    printf("Test Case 3:\\n");
    calculateQuarterlySales();
    
    printf("Test Case 4:\\n");
    calculateQuarterlySales();
    
    printf("Test Case 5:\\n");
    calculateQuarterlySales();
    
    return 0;
}
