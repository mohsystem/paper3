
#include <iostream>
#include <vector>

int countPassengers(const std::vector<std::vector<int>>& busStops) {
    // Input validation
    if (busStops.empty()) {
        return 0;
    }
    
    int peopleOnBus = 0;
    
    // Process each bus stop
    for (size_t i = 0; i < busStops.size(); i++) {
        // Validate array structure
        if (busStops[i].size() != 2) {
            continue;
        }
        
        // Validate non-negative values
        if (busStops[i][0] < 0 || busStops[i][1] < 0) {
            continue;
        }
        
        // Add people getting on, subtract people getting off
        peopleOnBus += busStops[i][0];
        peopleOnBus -= busStops[i][1];
        
        // Ensure non-negative passenger count
        if (peopleOnBus < 0) {
            peopleOnBus = 0;
        }
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1: Simple case
    std::vector<std::vector<int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test 1: " << countPassengers(test1) << std::endl; // Expected: 5
    
    // Test case 2: Multiple stops
    std::vector<std::vector<int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test 2: " << countPassengers(test2) << std::endl; // Expected: 17
    
    // Test case 3: Empty bus at end
    std::vector<std::vector<int>> test3 = {{5, 0}, {2, 3}, {0, 4}};
    std::cout << "Test 3: " << countPassengers(test3) << std::endl; // Expected: 0
    
    // Test case 4: Single stop
    std::vector<std::vector<int>> test4 = {{10, 0}};
    std::cout << "Test 4: " << countPassengers(test4) << std::endl; // Expected: 10
    
    // Test case 5: Multiple stops
    std::vector<std::vector<int>> test5 = {{3, 0}, {2, 1}, {5, 3}, {1, 4}};
    std::cout << "Test 5: " << countPassengers(test5) << std::endl; // Expected: 3
    
    return 0;
}
