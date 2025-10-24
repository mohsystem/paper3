#include <stdio.h>
#include <stddef.h> // for size_t

/**
 * @brief Calculates the number of people still on the bus after the last bus stop.
 * 
 * @param num_stops The number of bus stops (rows in the array).
 * @param stops A 2D array where each row represents [people_on, people_off].
 * @return The number of people remaining on the bus.
 */
int count_passengers(size_t num_stops, const int stops[][2]) {
    int passengers = 0;
    for (size_t i = 0; i < num_stops; i++) {
        passengers += stops[i][0];
        passengers -= stops[i][1];
    }
    return passengers;
}

int main() {
    // Test Case 1
    const int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    size_t size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test 1: %d\n", count_passengers(size1, test1)); // Expected: 5

    // Test Case 2
    const int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    size_t size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test 2: %d\n", count_passengers(size2, test2)); // Expected: 17

    // Test Case 3
    const int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    size_t size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test 3: %d\n", count_passengers(size3, test3)); // Expected: 21
    
    // Test Case 4
    const int test4[][2] = {{0, 0}};
    size_t size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test 4: %d\n", count_passengers(size4, test4)); // Expected: 0
    
    // Test Case 5
    const int test5[][2] = {{10, 0}, {1, 1}, {2, 2}, {3, 3}, {4, 10}};
    size_t size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test 5: %d\n", count_passengers(size5, test5)); // Expected: 4

    return 0;
}