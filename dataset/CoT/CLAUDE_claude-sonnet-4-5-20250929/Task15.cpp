
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

const int MAX_FLOORS = 1000;

std::vector<std::string> buildTower(int nFloors) {
    // Input validation
    if (nFloors <= 0) {
        return std::vector<std::string>();
    }
    if (nFloors > MAX_FLOORS) {
        throw std::invalid_argument("Number of floors exceeds maximum allowed");
    }
    
    std::vector<std::string> tower;
    tower.reserve(nFloors); // Pre-allocate memory for efficiency
    
    int width = 2 * nFloors - 1;
    
    for (int i = 0; i < nFloors; i++) {
        int stars = 2 * i + 1;
        int spaces = (width - stars) / 2;
        
        std::string floor;
        floor.reserve(width); // Pre-allocate to avoid reallocation
        
        // Add leading spaces
        floor.append(spaces, ' ');
        // Add stars
        floor.append(stars, '*');
        // Add trailing spaces
        floor.append(spaces, ' ');
        
        tower.push_back(floor);
    }
    
    return tower;
}

int main() {
    // Test case 1: 3 floors
    std::cout << "Test 1 - 3 floors:" << std::endl;
    std::vector<std::string> result1 = buildTower(3);
    for (const auto& floor : result1) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    
    // Test case 2: 6 floors
    std::cout << "\\nTest 2 - 6 floors:" << std::endl;
    std::vector<std::string> result2 = buildTower(6);
    for (const auto& floor : result2) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    
    // Test case 3: 1 floor
    std::cout << "\\nTest 3 - 1 floor:" << std::endl;
    std::vector<std::string> result3 = buildTower(1);
    for (const auto& floor : result3) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    
    // Test case 4: 0 floors (edge case)
    std::cout << "\\nTest 4 - 0 floors:" << std::endl;
    std::vector<std::string> result4 = buildTower(0);
    std::cout << "Empty array, length: " << result4.size() << std::endl;
    
    // Test case 5: 10 floors
    std::cout << "\\nTest 5 - 10 floors:" << std::endl;
    std::vector<std::string> result5 = buildTower(10);
    for (const auto& floor : result5) {
        std::cout << "\\"" << floor << "\\"" << std::endl;
    }
    
    return 0;
}
