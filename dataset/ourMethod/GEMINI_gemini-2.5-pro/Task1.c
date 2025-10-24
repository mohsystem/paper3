#include <stdio.h>
#include <stddef.h>

/**
 * @brief Calculates the number of people remaining on the bus after all stops.
 *
 * @param n The number of bus stops (i.e., the number of pairs in the array).
 * @param stops A 2D array where each row is a pair.
 *              stops[i][0] is the number of people getting on at stop i.
 *              stops[i][1] is the number of people getting off at stop i.
 * @return The total number of people on the bus after the last stop.
 */
int number(size_t n, const int stops[][2]) {
    int peopleOnBus = 0;
    for (size_t i = 0; i < n; ++i) {
        peopleOnBus += stops[i][0]; // People getting on
        peopleOnBus -= stops[i][1]; // People getting off
    }
    return peopleOnBus;
}

int main() {
    // Test Case 1
    const int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    size_t n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1: Expected 5, Got %d\n", number(n1, test1));

    // Test Case 2
    const int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    size_t n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2: Expected 17, Got %d\n", number(n2, test2));

    // Test Case 3
    const int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    size_t n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3: Expected 21, Got %d\n", number(n3, test3));

    // Test Case 4
    const int test4[][2] = {{0, 0}};
    size_t n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4: Expected 0, Got %d\n", number(n4, test4));

    // Test Case 5
    const int test5[][2] = {{100, 0}, {25, 50}, {10, 5}};
    size_t n5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5: Expected 80, Got %d\n", number(n5, test5));
    
    return 0;
}