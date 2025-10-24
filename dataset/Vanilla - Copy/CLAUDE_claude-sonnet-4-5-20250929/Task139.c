
#include <stdio.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

double calculateArea(double radius) {
    return M_PI * radius * radius;
}

int main() {
    // Test cases
    printf("Test Case 1 - Radius: 5.0\\n");
    printf("Area: %f\\n\\n", calculateArea(5.0));
    
    printf("Test Case 2 - Radius: 10.5\\n");
    printf("Area: %f\\n\\n", calculateArea(10.5));
    
    printf("Test Case 3 - Radius: 1.0\\n");
    printf("Area: %f\\n\\n", calculateArea(1.0));
    
    printf("Test Case 4 - Radius: 7.25\\n");
    printf("Area: %f\\n\\n", calculateArea(7.25));
    
    printf("Test Case 5 - Radius: 15.8\\n");
    printf("Area: %f\\n", calculateArea(15.8));
    
    return 0;
}
