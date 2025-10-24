
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Structure to represent a bus stop with people getting on and off
typedef struct {
    int on;  // people getting on
    int off; // people getting off
} BusStop;

// Function to calculate the number of people remaining on the bus
// Parameters:
//   stops: array of BusStop structures
//   size: number of stops in the array
// Returns: number of people still on the bus after the last stop, or -1 on error
// Security: validates all inputs to ensure non-negative values and prevent integer overflow
int peopleOnBus(const BusStop* stops, size_t size) {
    // Validate input pointer
    if (stops == NULL && size > 0) {
        fprintf(stderr, "Error: NULL pointer with non-zero size\\n");
        return -1;
    }
    
    // Validate input: empty list means no stops, return 0
    if (size == 0) {
        return 0;
    }
    
    // Validate size to prevent overflow in loop iterations
    if (size > INT_MAX) {
        fprintf(stderr, "Error: size too large\\n");
        return -1;
    }
    
    // Validate first stop: people getting off must be 0 (bus starts empty)
    if (stops[0].off != 0) {
        fprintf(stderr, "Error: First stop must have 0 people getting off\\n");
        return -1;
    }
    
    int peopleOnBus = 0;
    
    for (size_t i = 0; i < size; i++) {
        int on = stops[i].on;
        int off = stops[i].off;
        
        // Validate input: both values must be non-negative
        if (on < 0 || off < 0) {
            fprintf(stderr, "Error: Number of people cannot be negative at stop %zu\\n", i);
            return -1;
        }
        
        // Check for integer overflow when adding people getting on
        if (peopleOnBus > INT_MAX - on) {
            fprintf(stderr, "Error: Integer overflow at stop %zu\\n", i);
            return -1;
        }
        
        // Add people getting on
        peopleOnBus += on;
        
        // Validate: people getting off cannot exceed people on bus
        if (off > peopleOnBus) {
            fprintf(stderr, "Error: More people getting off than on bus at stop %zu\\n", i);
            return -1;
        }
        
        // Remove people getting off
        peopleOnBus -= off;
        
        // Sanity check: ensure non-negative result (should always be true due to prior checks)
        if (peopleOnBus < 0) {
            fprintf(stderr, "Error: Invalid state - negative people count at stop %zu\\n", i);
            return -1;
        }
    }
    
    return peopleOnBus;
}

int main(void) {
    // Test case 1: Basic scenario
    BusStop test1[] = {{10, 0}, {3, 5}, {5, 8}};
    int result1 = peopleOnBus(test1, sizeof(test1) / sizeof(test1[0]));
    printf("Test 1: %d (Expected: 5)\\n", result1);
    
    // Test case 2: Multiple stops
    BusStop test2[] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    int result2 = peopleOnBus(test2, sizeof(test2) / sizeof(test2[0]));
    printf("Test 2: %d (Expected: 17)\\n", result2);
    
    // Test case 3: Single stop
    BusStop test3[] = {{5, 0}};
    int result3 = peopleOnBus(test3, sizeof(test3) / sizeof(test3[0]));
    printf("Test 3: %d (Expected: 5)\\n", result3);
    
    // Test case 4: Multiple stops with varying numbers
    BusStop test4[] = {{10, 0}, {3, 5}, {2, 5}};
    int result4 = peopleOnBus(test4, sizeof(test4) / sizeof(test4[0]));
    printf("Test 4: %d (Expected: 5)\\n", result4);
    
    // Test case 5: Empty bus scenario
    BusStop test5[] = {{0, 0}};
    int result5 = peopleOnBus(test5, sizeof(test5) / sizeof(test5[0]));
    printf("Test 5: %d (Expected: 0)\\n", result5);
    
    return 0;
}
