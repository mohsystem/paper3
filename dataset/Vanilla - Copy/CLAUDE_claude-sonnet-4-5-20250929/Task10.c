
#include <stdio.h>

int getSum(int a, int b) {
    if (a == b) {
        return a;
    }
    
    int minVal = (a < b) ? a : b;
    int maxVal = (a > b) ? a : b;
    
    // Sum formula: n * (first + last) / 2
    int count = maxVal - minVal + 1;
    return count * (minVal + maxVal) / 2;
}

int main() {
    printf("%d\\n", getSum(1, 0));    // Expected: 1
    printf("%d\\n", getSum(1, 2));    // Expected: 3
    printf("%d\\n", getSum(0, 1));    // Expected: 1
    printf("%d\\n", getSum(1, 1));    // Expected: 1
    printf("%d\\n", getSum(-1, 0));   // Expected: -1
    printf("%d\\n", getSum(-1, 2));   // Expected: 2
    
    return 0;
}
