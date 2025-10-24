
#include <stdio.h>
#include <math.h>
#include <float.h>

// Function to calculate cube diagonal from volume
// Security: Validates input to prevent invalid mathematical operations
// Returns: Diagonal length rounded to 2 decimal places, or -1.0 on error
double cubeDiagonal(double volume) {
    // Input validation: volume must be positive and finite
    // Prevents: undefined behavior from negative/zero cube root, NaN, infinity
    if (!isfinite(volume)) {
        fprintf(stderr, "Error: Volume must be a finite number\\n");
        return -1.0;
    }
    if (volume <= 0.0) {
        fprintf(stderr, "Error: Volume must be positive\\n");
        return -1.0;
    }
    
    // Check for extremely large values that could cause overflow
    // Max safe volume to prevent numerical issues
    const double MAX_SAFE_VOLUME = 1e100;
    if (volume > MAX_SAFE_VOLUME) {
        fprintf(stderr, "Error: Volume too large\\n");
        return -1.0;
    }
    
    // Calculate side length: side = cube_root(volume)
    // Using cbrt for accurate cube root calculation
    double side = cbrt(volume);
    
    // Validate intermediate result
    if (!isfinite(side) || side <= 0.0) {
        fprintf(stderr, "Error: Invalid side calculation\\n");
        return -1.0;
    }
    
    // Calculate diagonal: diagonal = side * sqrt(3)
    // Main diagonal formula for cube: d = s * sqrt(3)
    double diagonal = side * sqrt(3.0);
    
    // Validate final result before rounding
    if (!isfinite(diagonal)) {
        fprintf(stderr, "Error: Invalid diagonal calculation\\n");
        return -1.0;
    }
    
    // Round to 2 decimal places: multiply by 100, round, divide by 100
    // Using round for proper rounding
    double rounded = round(diagonal * 100.0) / 100.0;
    
    return rounded;
}

int main(void) {
    // Test cases with error handling
    double result;
    
    // Test case 1: volume = 8
    result = cubeDiagonal(8.0);
    if (result >= 0.0) {
        printf("cubeDiagonal(8) = %.2f\\n", result);
    }
    
    // Test case 2: volume = 343
    result = cubeDiagonal(343.0);
    if (result >= 0.0) {
        printf("cubeDiagonal(343) = %.2f\\n", result);
    }
    
    // Test case 3: volume = 1157.625
    result = cubeDiagonal(1157.625);
    if (result >= 0.0) {
        printf("cubeDiagonal(1157.625) = %.2f\\n", result);
    }
    
    // Test case 4: volume = 1 (unit cube)
    result = cubeDiagonal(1.0);
    if (result >= 0.0) {
        printf("cubeDiagonal(1) = %.2f\\n", result);
    }
    
    // Test case 5: volume = 27
    result = cubeDiagonal(27.0);
    if (result >= 0.0) {
        printf("cubeDiagonal(27) = %.2f\\n", result);
    }
    
    return 0;
}
