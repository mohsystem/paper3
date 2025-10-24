#include <stdio.h>
#include <stddef.h> // For size_t

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A 2D array where each row represents a stop [on, off].
 *              The 'const' keyword ensures the function does not modify the input array.
 * @param num_stops The number of rows (stops) in the array.
 * @return The total number of people remaining on the bus.
 */
int countPassengers(const int stops[][2], size_t num_stops) {
    int peopleOnBus = 0;
    for (size_t i = 0; i < num_stops; ++i) {
        peopleOnBus += stops[i][0];
        peopleOnBus -= stops[i][1];
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    printf("%d\n", countPassengers(test1, sizeof(test1) / sizeof(test1[0])));

    // Test Case 2
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    printf("%d\n", countPassengers(test2, sizeof(test2) / sizeof(test2[0])));

    // Test Case 3: Single stop with no change
    int test3[][2] = {{0, 0}};
    printf("%d\n", countPassengers(test3, sizeof(test3) / sizeof(test3[0])));

    // Test Case 4: Ends with zero passengers
    int test4[][2] = {{100, 0}, {0, 50}, {25, 0}, {0, 75}};
    printf("%d\n", countPassengers(test4, sizeof(test4) / sizeof(test4[0])));

    // Test Case 5: Net change is zero at most stops
    int test5[][2] = {{5, 0}, {5, 5}, {5, 5}, {5, 5}, {5, 5}};
    printf("%d\n", countPassengers(test5, sizeof(test5) / sizeof(test5[0])));
    
    return 0;
}