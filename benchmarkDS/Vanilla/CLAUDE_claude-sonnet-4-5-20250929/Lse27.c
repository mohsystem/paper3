
#include <stdio.h>

int calculateTotalSales(int januarySales, int februarySales, int marchSales) {
    int totalSales = 0;
    totalSales += januarySales;
    totalSales += februarySales;
    totalSales += marchSales;
    return totalSales;
}

int main() {
    // Test case 1
    int result1 = calculateTotalSales(1000, 1500, 2000);
    printf("Total Sales: %d\\n", result1);
    
    // Test case 2
    int result2 = calculateTotalSales(500, 750, 1250);
    printf("Total Sales: %d\\n", result2);
    
    // Test case 3
    int result3 = calculateTotalSales(0, 0, 0);
    printf("Total Sales: %d\\n", result3);
    
    // Test case 4
    int result4 = calculateTotalSales(10000, 15000, 20000);
    printf("Total Sales: %d\\n", result4);
    
    // Test case 5
    int result5 = calculateTotalSales(250, 500, 750);
    printf("Total Sales: %d\\n", result5);
    
    return 0;
}
