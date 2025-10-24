
#include <stdio.h>
#include <stdlib.h>

int countPassengers(int busStops[][2], int numStops) {
    // Input validation
    if (busStops == NULL || numStops <= 0) {
        return 0;
    }
    
    int peopleOnBus = 0;
    
    // Process each bus stop
    for (int i = 0; i < numStops; i++) {
        // Validate non-negative values
        if (busStops[i][0] < 0 || busStops[i][1] < 0) {
            continue;
        }
        
        // Add people getting on, subtract people getting off
        peopleOnBus += busStops[i][0];
        peopleOnBus -= busStops[i][1];
        
        // Ensure non-negative passenger count
        if (peopleOnBus < 0) {
            peopleOnBus = 0;
        }
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1: Simple case
    int test1[][2] = {{10, 0}, {3, 5}, {5, 8}};
    printf("Test 1: %d\\n", countPassengers(test1, 3)); // Expected: 5
    
    // Test case 2: Multiple stops
    int test2[][2] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    printf("Test 2: %d\\n", countPassengers(test2, 6)); // Expected: 17
    
    // Test case 3: Empty bus at end
    int test3[][2] = {{5, 0}, {2, 3}, {0, 4}};
    printf("Test 3: %d\\n", countPassengers(test3, 3)); // Expected: 0
    
    // Test case 4: Single stop
    int test4[][2] = {{10, 0}};
    printf("Test 4: %d\\n", countPassengers(test4, 1)); // Expected: 10
    
    // Test case 5: Multiple stops
    int test5[][2] = {{3, 0}, {2, 1}, {5, 3}, {1, 4}};
    printf("Test 5: %d\\n", countPassengers(test5, 4)); // Expected: 3
    
    return 0;
}
