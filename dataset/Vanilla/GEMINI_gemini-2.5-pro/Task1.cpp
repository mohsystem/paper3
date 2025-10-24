#include <iostream>
#include <vector>
#include <utility> // For std::pair

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A vector of pairs. Each pair represents {people_on, people_off}.
 * @return The number of people left on the bus.
 */
int countPassengers(const std::vector<std::pair<int, int>>& stops) {
    int peopleOnBus = 0;
    for (const auto& stop : stops) {
        peopleOnBus += stop.first;  // People getting on
        peopleOnBus -= stop.second; // People getting off
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test Case 1: " << countPassengers(test1) << std::endl; // Expected: 5

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test Case 2: " << countPassengers(test2) << std::endl; // Expected: 17

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test Case 3: " << countPassengers(test3) << std::endl; // Expected: 21

    // Test Case 4
    std::vector<std::pair<int, int>> test4 = {{0, 0}};
    std::cout << "Test Case 4: " << countPassengers(test4) << std::endl; // Expected: 0

    // Test Case 5
    std::vector<std::pair<int, int>> test5 = {{100, 0}, {25, 10}, {30, 20}, {5, 40}};
    std::cout << "Test Case 5: " << countPassengers(test5) << std::endl; // Expected: 90

    return 0;
}