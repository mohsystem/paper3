
#include <stdio.h>

int countPassengers(int busStops[][2], int size) {
    int peopleOnBus = 0;
    
    for (int i = 0; i < size; i++) {
        peopleOnBus += busStops[i][0]; // people getting on
        peopleOnBus -= busStops[i][1]; // people getting off
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    printf("Test 1: %d\\n", countPassengers(test1, 3)); // Expected: 5
    
    // Test case 2
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    printf("Test 2: %d\\n", countPassengers(test2, 6)); // Expected: 17
    
    // Test case 3
    int test3[][2] = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    printf("Test 3: %d\\n", countPassengers(test3, 6)); // Expected: 21
    
    // Test case 4
    int test4[][2] = {{0, 0}};
    printf("Test 4: %d\\n", countPassengers(test4, 1)); // Expected: 0
    
    // Test case 5
    int test5[][2] = {{5, 0}, {2, 3}, {4, 1}};
    printf("Test 5: %d\\n", countPassengers(test5, 3)); // Expected: 7
    
    return 0;
}
