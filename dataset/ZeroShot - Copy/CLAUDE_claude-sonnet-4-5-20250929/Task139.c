
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double calculateCircleArea(double radius) {
    if (radius < 0) {
        fprintf(stderr, "Error: Radius cannot be negative\\n");
        return -1;
    }
    return M_PI * radius * radius;
}

int main() {
    double result;
    
    // Test cases
    printf("Test Case 1 - Radius: 5\\n");
    result = calculateCircleArea(5);
    if (result >= 0) printf("Area: %.6f\\n", result);
    
    printf("\\nTest Case 2 - Radius: 10.5\\n");
    result = calculateCircleArea(10.5);
    if (result >= 0) printf("Area: %.6f\\n", result);
    
    printf("\\nTest Case 3 - Radius: 0\\n");
    result = calculateCircleArea(0);
    if (result >= 0) printf("Area: %.6f\\n", result);
    
    printf("\\nTest Case 4 - Radius: 1\\n");
    result = calculateCircleArea(1);
    if (result >= 0) printf("Area: %.6f\\n", result);
    
    printf("\\nTest Case 5 - Radius: 7.5\\n");
    result = calculateCircleArea(7.5);
    if (result >= 0) printf("Area: %.6f\\n", result);
    
    // Uncomment to test negative radius
    // printf("\\nTest Case 6 - Radius: -5\\n");
    // result = calculateCircleArea(-5);
    // if (result >= 0) printf("Area: %.6f\\n", result);
    
    return 0;
}
