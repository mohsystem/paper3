#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Calculates the number of people remaining on the bus after all stops.
 *
 * @param stops A 2D array where each inner array represents a bus stop.
 *              stops[i][0] is the number of people getting on.
 *              stops[i][1] is the number of people getting off.
 * @param num_stops The number of stops (rows in the 2D array).
 * @return The number of people still on the bus.
 */
int number_of_people_on_bus(const int stops[][2], size_t num_stops) {
    int people_on_bus = 0;
    for (size_t i = 0; i < num_stops; ++i) {
        people_on_bus += stops[i][0];
        people_on_bus -= stops[i][1];
    }
    // The problem statement guarantees the number of people will not be negative.
    return people_on_bus;
}

int main() {
    // Test Case 1
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1: Result = %d, Expected = 5\n", number_of_people_on_bus(test1, size1));

    // Test Case 2
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2: Result = %d, Expected = 17\n", number_of_people_on_bus(test2, size2));

    // Test Case 3
    int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3: Result = %d, Expected = 21\n", number_of_people_on_bus(test3, size3));

    // Test Case 4 (Empty list)
    // The loop in the function will not execute if num_stops is 0,
    // correctly returning the initial value of 0.
    printf("Test Case 4: Result = %d, Expected = 0\n", number_of_people_on_bus(NULL, 0));

    // Test Case 5 (Single stop)
    int test5[][2] = {{5, 0}};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5: Result = %d, Expected = 5\n", number_of_people_on_bus(test5, size5));
    
    return 0;
}