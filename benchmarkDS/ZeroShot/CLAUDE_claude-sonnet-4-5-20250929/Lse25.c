
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int getMonthlySales() {
    return rand() % 100001;
}

void calculateQuarterlySales() {
    int q1Sales = 0, q2Sales = 0, q3Sales = 0, q4Sales = 0;
    
    // Quarter 1 (Jan, Feb, Mar)
    for (int i = 0; i < 3; i++) {
        q1Sales += getMonthlySales();
    }
    
    // Quarter 2 (Apr, May, Jun)
    for (int i = 0; i < 3; i++) {
        q2Sales += getMonthlySales();
    }
    
    // Quarter 3 (Jul, Aug, Sep)
    for (int i = 0; i < 3; i++) {
        q3Sales += getMonthlySales();
    }
    
    // Quarter 4 (Oct, Nov, Dec)
    for (int i = 0; i < 3; i++) {
        q4Sales += getMonthlySales();
    }
    
    printf("Quarter 1 Sales: $%d\\n", q1Sales);
    printf("Quarter 2 Sales: $%d\\n", q2Sales);
    printf("Quarter 3 Sales: $%d\\n", q3Sales);
    printf("Quarter 4 Sales: $%d\\n", q4Sales);
    printf("Total Annual Sales: $%d\\n", q1Sales + q2Sales + q3Sales + q4Sales);
}

int main() {
    srand(time(0));
    
    // Test case 1
    printf("Test Case 1:\\n");
    calculateQuarterlySales();
    
    // Test case 2
    printf("\\nTest Case 2:\\n");
    calculateQuarterlySales();
    
    // Test case 3
    printf("\\nTest Case 3:\\n");
    calculateQuarterlySales();
    
    // Test case 4
    printf("\\nTest Case 4:\\n");
    calculateQuarterlySales();
    
    // Test case 5
    printf("\\nTest Case 5:\\n");
    calculateQuarterlySales();
    
    return 0;
}
