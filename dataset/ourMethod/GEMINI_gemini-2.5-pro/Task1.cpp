#include <iostream>
#include <vector>
#include <utility>

/**
 * @brief Calculates the number of people remaining on the bus after all stops.
 *
 * @param busStops A vector of pairs, where each pair represents a bus stop.
 *                 The first element of the pair is the number of people getting on.
 *                 The second element is the number of people getting off.
 * @return The total number of people on the bus after the last stop.
 */
int number(const std::vector<std::pair<int, int>>& busStops) {
    int peopleOnBus = 0;
    for (const auto& stop : busStops) {
        peopleOnBus += stop.first;  // People getting on
        peopleOnBus -= stop.second; // People getting off
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test Case 1: Expected 5, Got " << number(test1) << std::endl;

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test Case 2: Expected 17, Got " << number(test2) << std::endl;

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test Case 3: Expected 21, Got " << number(test3) << std::endl;

    // Test Case 4
    std::vector<std::pair<int, int>> test4 = {{0, 0}};
    std::cout << "Test Case 4: Expected 0, Got " << number(test4) << std::endl;

    // Test Case 5
    std::vector<std::pair<int, int>> test5 = {{100, 0}, {25, 50}, {10, 5}};
    std::cout << "Test Case 5: Expected 80, Got " << number(test5) << std::endl;

    return 0;
}