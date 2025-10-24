#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * Calculates the number of people remaining on the bus after all stops.
 *
 * @param stops A 2D array where each row represents a stop and has two columns:
 *              [people_getting_on, people_getting_off].
 * @param num_stops The number of stops (rows) in the array.
 * @return The number of people on the bus after the last stop.
 */
int count_passengers(const int stops[][2], size_t num_stops) {
    // A check for a NULL pointer or zero stops is good practice.
    if (stops == NULL || num_stops == 0) {
        return 0;
    }
    
    int people_on_bus = 0;
    for (size_t i = 0; i < num_stops; i++) {
        people_on_bus += stops[i][0]; // People getting on
        people_on_bus -= stops[i][1]; // People getting off
    }
    return people_on_bus;
}

// In C, there are no classes like 'Task1'. The main function serves as the entry point.
int main() {
    // Test Case 1
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1: %d\n", count_passengers(test1, size1)); // Expected: 5

    // Test Case 2
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2: %d\n", count_passengers(test2, size2)); // Expected: 17

    // Test Case 3
    int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3: %d\n", count_passengers(test3, size3)); // Expected: 21

    // Test Case 4: A single stop with no activity
    int test4[][2] = {{0, 0}};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4: %d\n", count_passengers(test4, size4)); // Expected: 0

    // Test Case 5: Empty list of stops (by passing size 0)
    int test5[][2] = {};
    printf("Test Case 5: %d\n", count_passengers(test5, 0)); // Expected: 0

    return 0;
}