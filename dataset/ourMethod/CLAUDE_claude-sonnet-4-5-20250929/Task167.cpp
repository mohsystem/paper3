
#include <iostream>
#include <cmath>
#include <iomanip>
#include <stdexcept>
#include <limits>

// Function to calculate cube diagonal from volume
// Security: Validates input to prevent invalid mathematical operations
// Returns: Diagonal length rounded to 2 decimal places
double cubeDiagonal(double volume) {
    // Input validation: volume must be positive and finite
    // Prevents: undefined behavior from negative/zero cube root, NaN, infinity
    if (!std::isfinite(volume)) {
        throw std::invalid_argument("Volume must be a finite number");
    }
    if (volume <= 0.0) {
        throw std::invalid_argument("Volume must be positive");
    }
    
    // Check for extremely large values that could cause overflow
    // Max safe volume: approximately (DBL_MAX^(1/3))^3
    const double MAX_SAFE_VOLUME = 1e100;
    if (volume > MAX_SAFE_VOLUME) {
        throw std::invalid_argument("Volume too large");
    }
    
    // Calculate side length: side = cube_root(volume)
    // Using std::cbrt for accurate cube root calculation
    double side = std::cbrt(volume);
    
    // Validate intermediate result
    if (!std::isfinite(side) || side <= 0.0) {
        throw std::runtime_error("Invalid side calculation");
    }
    
    // Calculate diagonal: diagonal = side * sqrt(3)
    // Main diagonal formula for cube: d = s * sqrt(3)
    double diagonal = side * std::sqrt(3.0);
    
    // Validate final result before rounding
    if (!std::isfinite(diagonal)) {
        throw std::runtime_error("Invalid diagonal calculation");
    }
    
    // Round to 2 decimal places: multiply by 100, round, divide by 100
    // Using std::round for proper rounding (banker's rounding)
    double rounded = std::round(diagonal * 100.0) / 100.0;
    
    return rounded;
}

int main() {
    // Test cases with error handling
    std::cout << std::fixed << std::setprecision(2);
    
    try {
        // Test case 1: volume = 8
        double result1 = cubeDiagonal(8.0);
        std::cout << "cubeDiagonal(8) = " << result1 << std::endl;
        
        // Test case 2: volume = 343
        double result2 = cubeDiagonal(343.0);
        std::cout << "cubeDiagonal(343) = " << result2 << std::endl;
        
        // Test case 3: volume = 1157.625
        double result3 = cubeDiagonal(1157.625);
        std::cout << "cubeDiagonal(1157.625) = " << result3 << std::endl;
        
        // Test case 4: volume = 1 (unit cube)
        double result4 = cubeDiagonal(1.0);
        std::cout << "cubeDiagonal(1) = " << result4 << std::endl;
        
        // Test case 5: volume = 27
        double result5 = cubeDiagonal(27.0);
        std::cout << "cubeDiagonal(27) = " << result5 << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
