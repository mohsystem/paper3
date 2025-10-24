#include <iostream>
#include <vector>
#include <utility> // for std::pair

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A vector of pairs. Each pair represents {people_on, people_off}.
 * @return The number of people remaining on the bus.
 */
int countPassengers(const std::vector<std::pair<int, int>>& stops) {
    int passengers = 0;
    for (const auto& stop : stops) {
        passengers += stop.first;
        passengers -= stop.second;
    }
    return passengers;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test 1: " << countPassengers(test1) << std::endl; // Expected: 5

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test 2: " << countPassengers(test2) << std::endl; // Expected: 17

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test 3: " << countPassengers(test3) << std::endl; // Expected: 21
    
    // Test Case 4
    std::vector<std::pair<int, int>> test4 = {{0, 0}};
    std::cout << "Test 4: " << countPassengers(test4) << std::endl; // Expected: 0
    
    // Test Case 5
    std::vector<std::pair<int, int>> test5 = {{10, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 10}};
    std::cout << "Test 5: " << countPassengers(test5) << std::endl; // Expected: 4
    
    return 0;
}