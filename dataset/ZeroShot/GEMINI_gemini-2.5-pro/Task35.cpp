#include <iostream>
#include <vector>
#include <utility> // For std::pair

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A constant reference to a vector of pairs. Each pair represents a stop,
 *              containing the number of people getting on (first) and off (second).
 * @return The total number of people remaining on the bus.
 */
int countPassengers(const std::vector<std::pair<int, int>>& stops) {
    int peopleOnBus = 0;
    // Using a range-based for loop for safe and readable iteration.
    for (const auto& stop : stops) {
        peopleOnBus += stop.first;
        peopleOnBus -= stop.second;
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << countPassengers(test1) << std::endl;

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << countPassengers(test2) << std::endl;

    // Test Case 3: Single stop with no change
    std::vector<std::pair<int, int>> test3 = {{0, 0}};
    std::cout << countPassengers(test3) << std::endl;

    // Test Case 4: Ends with zero passengers
    std::vector<std::pair<int, int>> test4 = {{100, 0}, {0, 50}, {25, 0}, {0, 75}};
    std::cout << countPassengers(test4) << std::endl;

    // Test Case 5: Net change is zero at most stops
    std::vector<std::pair<int, int>> test5 = {{5, 0}, {5, 5}, {5, 5}, {5, 5}, {5, 5}};
    std::cout << countPassengers(test5) << std::endl;

    return 0;
}