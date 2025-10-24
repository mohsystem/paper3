
#include <stdio.h>
#include <stdlib.h>

int findGCD(int a, int b) {
    a = abs(a);
    b = abs(b);
    
    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

int main() {
    // Test case 1
    printf("GCD of 48 and 18: %d\\n", findGCD(48, 18));
    
    // Test case 2
    printf("GCD of 100 and 50: %d\\n", findGCD(100, 50));
    
    // Test case 3
    printf("GCD of 17 and 19: %d\\n", findGCD(17, 19));
    
    // Test case 4
    printf("GCD of 270 and 192: %d\\n", findGCD(270, 192));
    
    // Test case 5
    printf("GCD of 1071 and 462: %d\\n", findGCD(1071, 462));
    
    return 0;
}
