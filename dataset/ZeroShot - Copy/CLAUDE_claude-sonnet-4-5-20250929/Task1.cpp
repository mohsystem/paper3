
#include <iostream>
#include <vector>

int countPassengers(const std::vector<std::vector<int>>& busStops) {
    int peopleOnBus = 0;
    
    for (size_t i = 0; i < busStops.size(); i++) {
        peopleOnBus += busStops[i][0]; // people getting on
        peopleOnBus -= busStops[i][1]; // people getting off
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1
    std::vector<std::vector<int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test 1: " << countPassengers(test1) << std::endl; // Expected: 5
    
    // Test case 2
    std::vector<std::vector<int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test 2: " << countPassengers(test2) << std::endl; // Expected: 17
    
    // Test case 3
    std::vector<std::vector<int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test 3: " << countPassengers(test3) << std::endl; // Expected: 21
    
    // Test case 4
    std::vector<std::vector<int>> test4 = {{0, 0}};
    std::cout << "Test 4: " << countPassengers(test4) << std::endl; // Expected: 0
    
    // Test case 5
    std::vector<std::vector<int>> test5 = {{5, 0}, {2, 3}, {4, 1}, {3, 2}, {1, 4}};
    std::cout << "Test 5: " << countPassengers(test5) << std::endl; // Expected: 5
    
    return 0;
}
