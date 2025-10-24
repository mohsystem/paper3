
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <sstream>

// Function to calculate circle area
// Returns true on success, false on invalid input
bool calculateCircleArea(double radius, double& area) {
    // Validate input: radius must be non-negative and finite
    if (radius < 0.0) {
        return false; // Negative radius is invalid
    }
    
    // Check for infinity and NaN to prevent undefined behavior
    if (std::isnan(radius) || std::isinf(radius)) {
        return false; // Invalid numeric value
    }
    
    // Prevent potential overflow in calculation
    // Max safe radius: sqrt(DBL_MAX / PI)
    const double MAX_SAFE_RADIUS = std::sqrt(std::numeric_limits<double>::max() / M_PI);
    if (radius > MAX_SAFE_RADIUS) {
        return false; // Radius too large, would cause overflow
    }
    
    // Calculate area: A = π * r²
    area = M_PI * radius * radius;
    
    // Verify result is valid (no overflow occurred)
    if (std::isnan(area) || std::isinf(area)) {
        return false; // Calculation resulted in invalid value
    }
    
    return true;
}

// Safe input reading with validation
bool readRadius(double& radius) {
    std::string input;
    
    // Read entire line to prevent buffer issues
    if (!std::getline(std::cin, input)) {
        return false; // Input stream error
    }
    
    // Validate input is not empty
    if (input.empty()) {
        return false;
    }
    
    // Validate input length to prevent excessive processing
    const size_t MAX_INPUT_LENGTH = 100;
    if (input.length() > MAX_INPUT_LENGTH) {
        return false; // Input too long
    }
    
    // Parse using stringstream for safe conversion
    std::istringstream iss(input);
    double value;
    
    // Try to extract double value
    if (!(iss >> value)) {
        return false; // Not a valid number
    }
    
    // Ensure no extra characters after the number
    std::string remaining;
    if (iss >> remaining) {
        return false; // Extra characters found
    }
    
    radius = value;
    return true;
}

int main() {
    // Test cases
    struct TestCase {
        double radius;
        bool expectValid;
    };
    
    TestCase tests[] = {
        {5.0, true},        // Valid positive radius
        {0.0, true},        // Valid zero radius
        {10.5, true},       // Valid decimal radius
        {-3.0, false},      // Invalid negative radius
        {100.0, true}       // Valid large radius
    };
    
    std::cout << "Running test cases:\\n";
    for (size_t i = 0; i < 5; ++i) {
        double area = 0.0;
        bool result = calculateCircleArea(tests[i].radius, area);
        
        std::cout << "Test " << (i + 1) << ": radius = " << tests[i].radius;
        
        if (result && tests[i].expectValid) {
            std::cout << " -> area = " << area << " (PASS)\\n";
        } else if (!result && !tests[i].expectValid) {
            std::cout << " -> Invalid input (PASS)\\n";
        } else {
            std::cout << " -> FAIL\\n";
        }
    }
    
    // Interactive mode
    std::cout << "\\nEnter radius (or Ctrl+D to exit): ";
    double radius;
    
    while (readRadius(radius)) {
        double area = 0.0;
        
        if (calculateCircleArea(radius, area)) {
            std::cout << "Area of circle with radius " << radius 
                      << " is " << area << std::endl;
        } else {
            std::cout << "Error: Invalid radius. Must be non-negative and finite." << std::endl;
        }
        
        std::cout << "Enter radius (or Ctrl+D to exit): ";
    }
    
    return 0;
}
