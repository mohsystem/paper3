
#include <stdio.h>
#include <math.h>
#include <float.h>

double cubeDiagonal(double volume) {
    // Input validation
    if (volume <= 0 || isnan(volume) || isinf(volume)) {
        fprintf(stderr, "Error: Volume must be a positive finite number\\n");
        return -1.0;
    }
    
    // Calculate side length from volume
    double side = cbrt(volume);
    
    // Calculate main diagonal: side * sqrt(3)
    double diagonal = side * sqrt(3.0);
    
    // Round to 2 decimal places
    return round(diagonal * 100.0) / 100.0;
}

int main() {
    // Test cases
    printf("Test 1: %.2f\\n", cubeDiagonal(8));           // Expected: 3.46
    printf("Test 2: %.2f\\n", cubeDiagonal(343));         // Expected: 12.12
    printf("Test 3: %.2f\\n", cubeDiagonal(1157.625));    // Expected: 18.19
    printf("Test 4: %.2f\\n", cubeDiagonal(1));           // Expected: 1.73
    printf("Test 5: %.2f\\n", cubeDiagonal(27));          // Expected: 5.20
    
    return 0;
}
