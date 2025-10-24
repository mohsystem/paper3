#include <iostream>
#include <vector>
#include <utility> // For std::pair

// The main class is named Task1 as requested.
class Task1 {
public:
    /**
     * Calculates the number of people remaining on the bus after all stops.
     *
     * @param stops A vector of pairs. Each pair represents a stop
     *              with {people_getting_on, people_getting_off}.
     * @return The number of people on the bus after the last stop.
     */
    static int countPassengers(const std::vector<std::pair<int, int>>& stops) {
        int peopleOnBus = 0;
        for (const auto& stop : stops) {
            peopleOnBus += stop.first;  // People getting on
            peopleOnBus -= stop.second; // People getting off
        }
        return peopleOnBus;
    }
};

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test Case 1: " << Task1::countPassengers(test1) << std::endl; // Expected: 5

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test Case 2: " << Task1::countPassengers(test2) << std::endl; // Expected: 17

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test Case 3: " << Task1::countPassengers(test3) << std::endl; // Expected: 21

    // Test Case 4: A single stop with no activity
    std::vector<std::pair<int, int>> test4 = {{0, 0}};
    std::cout << "Test Case 4: " << Task1::countPassengers(test4) << std::endl; // Expected: 0

    // Test Case 5: Empty vector of stops
    std::vector<std::pair<int, int>> test5 = {};
    std::cout << "Test Case 5: " << Task1::countPassengers(test5) << std::endl; // Expected: 0

    return 0;
}