
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <errno.h>

#define PI 3.14159265358979323846

typedef enum {
    SUCCESS = 0,
    ERROR_NEGATIVE_RADIUS = -1,
    ERROR_INVALID_VALUE = -2,
    ERROR_OVERFLOW = -3
} ErrorCode;

ErrorCode calculateCircleArea(double radius, double* area) {
    // Input validation
    if (area == NULL) {
        return ERROR_INVALID_VALUE;
    }
    
    if (radius < 0) {
        return ERROR_NEGATIVE_RADIUS;
    }
    
    if (isnan(radius) || isinf(radius)) {
        return ERROR_INVALID_VALUE;
    }
    
    // Check for potential overflow
    double max_radius = sqrt(DBL_MAX / PI);
    if (radius > max_radius) {
        return ERROR_OVERFLOW;
    }
    
    // Calculate area
    *area = PI * radius * radius;
    
    if (isinf(*area)) {
        return ERROR_OVERFLOW;
    }
    
    return SUCCESS;
}

const char* getErrorMessage(ErrorCode code) {
    switch (code) {
        case SUCCESS:
            return "Success";
        case ERROR_NEGATIVE_RADIUS:
            return "Radius cannot be negative";
        case ERROR_INVALID_VALUE:
            return "Invalid radius value";
        case ERROR_OVERFLOW:
            return "Radius too large, calculation would overflow";
        default:
            return "Unknown error";
    }
}

int main() {
    double area;
    ErrorCode result;
    
    // Test Case 1
    printf("Test Case 1 - Radius: 5.0\\n");
    result = calculateCircleArea(5.0, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    // Test Case 2
    printf("\\nTest Case 2 - Radius: 10.5\\n");
    result = calculateCircleArea(10.5, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    // Test Case 3
    printf("\\nTest Case 3 - Radius: 0.0\\n");
    result = calculateCircleArea(0.0, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    // Test Case 4
    printf("\\nTest Case 4 - Radius: 1.0\\n");
    result = calculateCircleArea(1.0, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    // Test Case 5
    printf("\\nTest Case 5 - Radius: 7.5\\n");
    result = calculateCircleArea(7.5, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    // Test error case
    printf("\\nTest Case 6 - Radius: -5.0 (Error case)\\n");
    result = calculateCircleArea(-5.0, &area);
    if (result == SUCCESS) {
        printf("Area: %.6f\\n", area);
    } else {
        printf("Error: %s\\n", getErrorMessage(result));
    }
    
    return 0;
}
