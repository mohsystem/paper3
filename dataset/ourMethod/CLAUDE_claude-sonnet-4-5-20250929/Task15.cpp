
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

// Security: Validate input to prevent resource exhaustion and integer overflow
// Maximum reasonable tower size to prevent excessive memory allocation
const int MAX_FLOORS = 10000;

std::vector<std::string> buildTower(int nFloors) {
    // Security: Validate input range to prevent negative values and resource exhaustion
    if (nFloors <= 0) {
        throw std::invalid_argument("Number of floors must be positive");
    }
    if (nFloors > MAX_FLOORS) {
        throw std::invalid_argument("Number of floors exceeds maximum allowed");
    }
    
    // Security: Check for integer overflow before calculating width
    // Width formula: 2 * nFloors - 1
    if (nFloors > (std::numeric_limits<int>::max() / 2)) {
        throw std::overflow_error("Floor calculation would cause integer overflow");
    }
    
    int width = 2 * nFloors - 1;
    std::vector<std::string> tower;
    
    // Security: Reserve exact capacity to prevent reallocation attacks
    tower.reserve(nFloors);
    
    // Build each floor from top to bottom
    for (int i = 0; i < nFloors; ++i) {
        // Security: Check for integer overflow in stars calculation
        int stars = 2 * i + 1;
        if (i > 0 && stars < 0) {
            throw std::overflow_error("Stars calculation overflow");
        }
        
        int spaces = (width - stars) / 2;
        
        // Security: Validate calculated values are non-negative
        if (spaces < 0 || stars < 0) {
            throw std::logic_error("Invalid floor dimensions calculated");
        }
        
        // Security: Use string constructor with count to prevent buffer issues
        std::string floor;
        floor.reserve(width); // Pre-allocate to prevent reallocations
        floor.append(spaces, ' ');
        floor.append(stars, '*');
        floor.append(spaces, ' ');
        
        tower.push_back(floor);
    }
    
    return tower;
}

int main() {
    // Test case 1: Basic 3-floor tower
    try {
        std::vector<std::string> tower1 = buildTower(3);
        std::cout << "Test 1 - 3 floors:\\n";
        for (const auto& floor : tower1) {
            std::cout << "\\"" << floor << "\\"\\n";
        }
        std::cout << "\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << "\\n";
    }
    
    // Test case 2: 6-floor tower
    try {
        std::vector<std::string> tower2 = buildTower(6);
        std::cout << "Test 2 - 6 floors:\\n";
        for (const auto& floor : tower2) {
            std::cout << "\\"" << floor << "\\"\\n";
        }
        std::cout << "\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << "\\n";
    }
    
    // Test case 3: Single floor
    try {
        std::vector<std::string> tower3 = buildTower(1);
        std::cout << "Test 3 - 1 floor:\\n";
        for (const auto& floor : tower3) {
            std::cout << "\\"" << floor << "\\"\\n";
        }
        std::cout << "\\n";
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << "\\n";
    }
    
    // Test case 4: Invalid input (zero)
    try {
        std::vector<std::string> tower4 = buildTower(0);
        std::cout << "Test 4 - 0 floors:\\n";
        for (const auto& floor : tower4) {
            std::cout << "\\"" << floor << "\\"\\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Expected error: " << e.what() << "\\n\\n";
    }
    
    // Test case 5: Invalid input (negative)
    try {
        std::vector<std::string> tower5 = buildTower(-5);
        std::cout << "Test 5 - negative floors:\\n";
        for (const auto& floor : tower5) {
            std::cout << "\\"" << floor << "\\"\\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Expected error: " << e.what() << "\\n\\n";
    }
    
    return 0;
}
