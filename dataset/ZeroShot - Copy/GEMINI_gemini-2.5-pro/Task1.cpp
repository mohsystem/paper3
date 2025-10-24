#include <iostream>
#include <vector>
#include <utility> // for std::pair

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A vector of pairs, where each pair represents {people_on, people_off}.
 * @return The number of people remaining on the bus.
 */
int countPassengers(const std::vector<std::pair<int, int>>& stops) {
    int passengers = 0;
    for (const auto& stop : stops) {
        passengers += stop.first;  // People getting on
        passengers -= stop.second; // People getting off
    }
    return passengers;
}

void runTestCase(int caseNum, const std::vector<std::pair<int, int>>& test_data) {
    std::cout << "Test Case " << caseNum << ": " << countPassengers(test_data) << std::endl;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    runTestCase(1, test1); // Expected: 5

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    runTestCase(2, test2); // Expected: 17

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    runTestCase(3, test3); // Expected: 21

    // Test Case 4
    std::vector<std::pair<int, int>> test4 = {{0, 0}};
    runTestCase(4, test4); // Expected: 0

    // Test Case 5
    std::vector<std::pair<int, int>> test5 = {{100, 0}, {50, 25}, {30, 90}};
    runTestCase(5, test5); // Expected: 65

    return 0;
}