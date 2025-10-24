#include <iostream>
#include <vector>
#include <utility> // For std::pair

/**
 * @brief Calculates the number of people remaining on the bus after all stops.
 *
 * @param stops A vector of pairs, where each pair represents a bus stop.
 *              The first element of the pair is the number of people getting on.
 *              The second element is the number of people getting off.
 * @return The number of people still on the bus.
 */
int number_of_people_on_bus(const std::vector<std::pair<int, int>>& stops) {
    int people_on_bus = 0;
    for (const auto& stop : stops) {
        people_on_bus += stop.first;
        people_on_bus -= stop.second;
    }
    // The problem statement guarantees the number of people will not be negative.
    return people_on_bus;
}

int main() {
    // Test Case 1
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test Case 1: Result = " << number_of_people_on_bus(test1) << ", Expected = 5" << std::endl;

    // Test Case 2
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test Case 2: Result = " << number_of_people_on_bus(test2) << ", Expected = 17" << std::endl;

    // Test Case 3
    std::vector<std::pair<int, int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    std::cout << "Test Case 3: Result = " << number_of_people_on_bus(test3) << ", Expected = 21" << std::endl;

    // Test Case 4 (Empty list)
    std::vector<std::pair<int, int>> test4 = {};
    std::cout << "Test Case 4: Result = " << number_of_people_on_bus(test4) << ", Expected = 0" << std::endl;

    // Test Case 5 (Single stop)
    std::vector<std::pair<int, int>> test5 = {{5, 0}};
    std::cout << "Test Case 5: Result = " << number_of_people_on_bus(test5) << ", Expected = 5" << std::endl;

    return 0;
}