
#include <stdio.h>

int getSum(int a, int b) {
    // Handle case where a and b are equal
    if (a == b) {
        return a;
    }
    
    // Ensure min and max are correctly ordered
    int minVal = (a < b) ? a : b;
    int maxVal = (a > b) ? a : b;
    
    // Use arithmetic series formula: sum = n * (first + last) / 2
    // where n is the count of numbers
    int count = maxVal - minVal + 1;
    return count * (minVal + maxVal) / 2;
}

int main() {
    // Test cases
    printf("%d\\n", getSum(1, 0));    // Expected: 1
    printf("%d\\n", getSum(1, 2));    // Expected: 3
    printf("%d\\n", getSum(0, 1));    // Expected: 1
    printf("%d\\n", getSum(1, 1));    // Expected: 1
    printf("%d\\n", getSum(-1, 0));   // Expected: -1
    printf("%d\\n", getSum(-1, 2));   // Expected: 2
    
    return 0;
}
