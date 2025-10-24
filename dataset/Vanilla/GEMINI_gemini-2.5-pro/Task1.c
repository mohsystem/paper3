#include <stdio.h>

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A 2D array where each row is a pair [people_on, people_off].
 * @param num_stops The number of stops (rows in the array).
 * @return The number of people left on the bus.
 */
int countPassengers(const int stops[][2], int num_stops) {
    int peopleOnBus = 0;
    for (int i = 0; i < num_stops; ++i) {
        peopleOnBus += stops[i][0]; // People getting on
        peopleOnBus -= stops[i][1]; // People getting off
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    const int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    int n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1: %d\n", countPassengers(test1, n1)); // Expected: 5

    // Test Case 2
    const int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    int n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2: %d\n", countPassengers(test2, n2)); // Expected: 17

    // Test Case 3
    const int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    int n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3: %d\n", countPassengers(test3, n3)); // Expected: 21

    // Test Case 4
    const int test4[][2] = {{0, 0}};
    int n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4: %d\n", countPassengers(test4, n4)); // Expected: 0

    // Test Case 5
    const int test5[][2] = {{100, 0}, {25, 10}, {30, 20}, {5, 40}};
    int n5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5: %d\n", countPassengers(test5, n5)); // Expected: 90

    return 0;
}