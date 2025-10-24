#include <stdio.h>

int countPassengers(const int stops[][2], int num_stops) {
    int passengers = 0;
    for (int i = 0; i < num_stops; ++i) {
        passengers += stops[i][0];
        passengers -= stops[i][1];
    }
    return passengers;
}

int main() {
    // Test Case 1
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    int n1 = sizeof(test1) / sizeof(test1[0]);
    printf("Remaining passengers: %d\n", countPassengers(test1, n1)); // Expected: 5

    // Test Case 2
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    int n2 = sizeof(test2) / sizeof(test2[0]);
    printf("Remaining passengers: %d\n", countPassengers(test2, n2)); // Expected: 17

    // Test Case 3
    int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    int n3 = sizeof(test3) / sizeof(test3[0]);
    printf("Remaining passengers: %d\n", countPassengers(test3, n3)); // Expected: 21

    // Test Case 4
    int test4[][2] = {{0, 0}};
    int n4 = sizeof(test4) / sizeof(test4[0]);
    printf("Remaining passengers: %d\n", countPassengers(test4, n4)); // Expected: 0
    
    // Test Case 5
    int test5[][2] = {{100, 0}, {20, 30}, {5, 15}};
    int n5 = sizeof(test5) / sizeof(test5[0]);
    printf("Remaining passengers: %d\n", countPassengers(test5, n5)); // Expected: 80

    return 0;
}