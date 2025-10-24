#include <stdio.h>

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param stops A 2D array where each inner array is a pair of [people_on, people_off].
 * @param num_stops The number of stops (rows in the array).
 * @return The number of people remaining on the bus.
 */
int countPassengers(const int stops[][2], int num_stops) {
    int passengers = 0;
    for (int i = 0; i < num_stops; ++i) {
        passengers += stops[i][0]; // People getting on
        passengers -= stops[i][1]; // People getting off
    }
    return passengers;
}

void runTestCase(int caseNum, const int stops[][2], int num_stops) {
    printf("Test Case %d: %d\n", caseNum, countPassengers(stops, num_stops));
}

int main() {
    // Test Case 1
    const int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    int num_stops1 = sizeof(test1) / sizeof(test1[0]);
    runTestCase(1, test1, num_stops1); // Expected: 5

    // Test Case 2
    const int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    int num_stops2 = sizeof(test2) / sizeof(test2[0]);
    runTestCase(2, test2, num_stops2); // Expected: 17
    
    // Test Case 3
    const int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    int num_stops3 = sizeof(test3) / sizeof(test3[0]);
    runTestCase(3, test3, num_stops3); // Expected: 21

    // Test Case 4
    const int test4[][2] = {{0, 0}};
    int num_stops4 = sizeof(test4) / sizeof(test4[0]);
    runTestCase(4, test4, num_stops4); // Expected: 0

    // Test Case 5
    const int test5[][2] = {{100, 0}, {50, 25}, {30, 90}};
    int num_stops5 = sizeof(test5) / sizeof(test5[0]);
    runTestCase(5, test5, num_stops5); // Expected: 65

    return 0;
}