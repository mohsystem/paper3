
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Structure to represent a bus stop with people on and off
typedef struct {
    int people_on;
    int people_off;
} BusStop;

// Security: Validate and compute number of people on bus
// Returns -1 on error, otherwise returns valid people count
int numberOfPeopleOnBus(const BusStop* busStops, size_t numStops) {
    // Security: Validate input pointer is not NULL
    if (busStops == NULL) {
        fprintf(stderr, "Error: Bus stops array is NULL\\n");
        return -1;
    }
    
    // Security: Validate array has at least one stop
    if (numStops == 0) {
        fprintf(stderr, "Error: Bus stops array is empty\\n");
        return -1;
    }
    
    // Security: Validate SIZE_MAX won't cause issues\n    if (numStops > SIZE_MAX / sizeof(BusStop)) {\n        fprintf(stderr, "Error: Invalid array size\\n");\n        return -1;\n    }\n    \n    // Security: Validate first stop has 0 people getting off\n    if (busStops[0].people_off != 0) {\n        fprintf(stderr, "Error: First bus stop must have 0 people getting off\\n");\n        return -1;\n    }\n    \n    int peopleOnBus = 0;\n    \n    // Process each bus stop\n    for (size_t i = 0; i < numStops; i++) {\n        int peopleOn = busStops[i].people_on;\n        int peopleOff = busStops[i].people_off;\n        \n        // Security: Validate non-negative values\n        if (peopleOn < 0 || peopleOff < 0) {\n            fprintf(stderr, "Error: People count cannot be negative at stop %zu\\n", i);\n            return -1;\n        }\n        \n        // Security: Check for integer overflow when adding people on\n        if (peopleOnBus > INT_MAX - peopleOn) {\n            fprintf(stderr, "Error: Integer overflow at stop %zu\\n", i);\n            return -1;\n        }\n        \n        // Security: Ensure we don't have more people getting off than on bus
        if (peopleOff > peopleOnBus + peopleOn) {
            fprintf(stderr, "Error: More people getting off than on bus at stop %zu\\n", i);
            return -1;
        }
        
        // Update bus count
        peopleOnBus += peopleOn;
        peopleOnBus -= peopleOff;
        
        // Security: Defensive check for negative count
        if (peopleOnBus < 0) {
            fprintf(stderr, "Error: Bus people count became negative at stop %zu\\n", i);
            return -1;
        }
    }
    
    return peopleOnBus;
}

int main(void) {
    // Test case 1: Simple scenario
    BusStop test1[] = {{10, 0}, {3, 5}, {5, 8}};
    int result1 = numberOfPeopleOnBus(test1, sizeof(test1) / sizeof(test1[0]));
    printf("Test 1: %d (expected: 5)\\n", result1);
    
    // Test case 2: Multiple stops
    BusStop test2[] = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    int result2 = numberOfPeopleOnBus(test2, sizeof(test2) / sizeof(test2[0]));
    printf("Test 2: %d (expected: 17)\\n", result2);
    
    // Test case 3: Single stop
    BusStop test3[] = {{5, 0}};
    int result3 = numberOfPeopleOnBus(test3, sizeof(test3) / sizeof(test3[0]));
    printf("Test 3: %d (expected: 5)\\n", result3);
    
    // Test case 4: Multiple stops with various on/off
    BusStop test4[] = {{10, 0}, {3, 5}, {2, 5}};
    int result4 = numberOfPeopleOnBus(test4, sizeof(test4) / sizeof(test4[0]));
    printf("Test 4: %d (expected: 5)\\n", result4);
    
    // Test case 5: Everyone gets off at last stop
    BusStop test5[] = {{1, 0}, {2, 0}, {3, 0}, {4, 10}};
    int result5 = numberOfPeopleOnBus(test5, sizeof(test5) / sizeof(test5[0]));
    printf("Test 5: %d (expected: 0)\\n", result5);
    
    return 0;
}
