
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <float.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Function to calculate circle area
// Returns true on success, false on invalid input
bool calculateCircleArea(double radius, double* area) {
    // Validate output pointer to prevent null pointer dereference
    if (area == NULL) {
        return false;
    }
    
    // Initialize output to safe default
    *area = 0.0;
    
    // Validate input: radius must be non-negative and finite
    if (radius < 0.0) {
        return false; // Negative radius is invalid
    }
    
    // Check for infinity and NaN to prevent undefined behavior
    if (isnan(radius) || isinf(radius)) {
        return false; // Invalid numeric value
    }
    
    // Prevent potential overflow in calculation
    // Max safe radius: sqrt(DBL_MAX / PI)
    const double MAX_SAFE_RADIUS = sqrt(DBL_MAX / M_PI);
    if (radius > MAX_SAFE_RADIUS) {
        return false; // Radius too large, would cause overflow
    }
    
    // Calculate area: A = π * r²
    *area = M_PI * radius * radius;
    
    // Verify result is valid (no overflow occurred)
    if (isnan(*area) || isinf(*area)) {
        *area = 0.0; // Reset to safe value
        return false; // Calculation resulted in invalid value
    }
    
    return true;
}

// Safe input reading with validation
bool readRadius(double* radius) {
    // Validate output pointer
    if (radius == NULL) {
        return false;
    }
    
    // Initialize output
    *radius = 0.0;
    
    // Use fixed-size buffer with bounds checking
    char buffer[128];
    
    // Initialize buffer to zeros for safety
    memset(buffer, 0, sizeof(buffer));
    
    // Read input with size limit (prevent buffer overflow)
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false; // Input error or EOF
    }
    
    // Validate buffer contains data
    size_t len = strlen(buffer);
    if (len == 0) {
        return false;
    }
    
    // Remove newline if present
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    // Check if input was truncated (line too long)
    if (len == sizeof(buffer) - 1 && buffer[len - 1] != '\\0') {
        // Clear remaining input
        int c;
        while ((c = getchar()) != '\\n' && c != EOF);
        return false;
    }
    
    // Validate string is not empty after removing newline
    if (len == 0) {
        return false;
    }
    
    // Parse string to double using safe conversion
    char* endptr = NULL;
    double value = strtod(buffer, &endptr);
    
    // Check for conversion errors
    if (endptr == buffer) {
        return false; // No conversion performed
    }
    
    // Check for extra characters after number
    while (*endptr != '\\0') {
        if (!isspace((unsigned char)*endptr)) {
            return false; // Non-whitespace character found
        }
        endptr++;
    }
    
    *radius = value;
    return true;
}

int main(void) {
    // Test cases
    struct TestCase {
        double radius;
        bool expectValid;
    };
    
    struct TestCase tests[] = {
        {5.0, true},        // Valid positive radius
        {0.0, true},        // Valid zero radius
        {10.5, true},       // Valid decimal radius
        {-3.0, false},      // Invalid negative radius
        {100.0, true}       // Valid large radius
    };
    
    printf("Running test cases:\\n");
    
    size_t numTests = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < numTests; i++) {
        double area = 0.0;
        bool result = calculateCircleArea(tests[i].radius, &area);
        
        printf("Test %zu: radius = %.2f", i + 1, tests[i].radius);
        
        if (result && tests[i].expectValid) {
            printf(" -> area = %.6f (PASS)\\n", area);
        } else if (!result && !tests[i].expectValid) {
            printf(" -> Invalid input (PASS)\\n");
        } else {
            printf(" -> FAIL\\n");
        }
    }
    
    // Interactive mode
    printf("\\nEnter radius (or Ctrl+D to exit): ");
    double radius;
    
    while (readRadius(&radius)) {
        double area = 0.0;
        
        if (calculateCircleArea(radius, &area)) {
            printf("Area of circle with radius %.2f is %.6f\\n", radius, area);
        } else {
            printf("Error: Invalid radius. Must be non-negative and finite.\\n");
        }
        
        printf("Enter radius (or Ctrl+D to exit): ");
    }
    
    printf("\\n");
    return 0;
}
