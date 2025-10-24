
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// Function to validate if a 10-minute walk returns to starting point
// Security: All inputs validated, bounds checked, safe string operations
// Returns true if walk takes exactly 10 minutes and returns to origin
bool isValidWalk(const char* walk[], size_t walkSize) {
    // Security: Validate input pointer is not NULL
    if (walk == NULL) {
        return false;
    }
    
    // Validate input: must be exactly 10 directions for 10 minutes
    // Security: Check size to prevent processing invalid input
    if (walkSize != 10) {
        return false;
    }
    
    // Track position on grid: x (east/west), y (north/south)
    int x = 0;
    int y = 0;
    
    // Process each direction
    for (size_t i = 0; i < walkSize; i++) {
        // Security: Validate each direction pointer is not NULL
        if (walk[i] == NULL) {
            return false;
        }
        
        // Security: Validate string length using strnlen with limit
        // Prevents reading beyond buffer bounds
        size_t len = strnlen(walk[i], 2);
        
        // Security: Validate only single character directions
        if (len != 1) {
            return false;
        }
        
        char direction = walk[i][0];
        
        // Security: Validate direction is one of expected values
        // Update position based on validated direction
        switch (direction) {
            case 'n':
                y++;
                break;
            case 's':
                y--;
                break;
            case 'e':
                x++;
                break;
            case 'w':
                x--;
                break;
            default:
                // Security: Reject invalid direction characters
                return false;
        }
    }
    
    // Check if we returned to starting point (0, 0)
    return (x == 0 && y == 0);
}

int main(void) {
    // Test case 1: Valid 10-minute walk returning to origin
    const char* test1[] = {"n", "s", "n", "s", "n", "s", "n", "s", "n", "s"};
    printf("Test 1: %s\\n", isValidWalk(test1, 10) ? "true" : "false");
    
    // Test case 2: Valid walk with all directions
    const char* test2[] = {"n", "e", "s", "w", "n", "e", "s", "w", "n", "s"};
    printf("Test 2: %s\\n", isValidWalk(test2, 10) ? "true" : "false");
    
    // Test case 3: Too short (less than 10 minutes)
    const char* test3[] = {"n", "s", "e", "w"};
    printf("Test 3: %s\\n", isValidWalk(test3, 4) ? "true" : "false");
    
    // Test case 4: Correct length but doesn't return to origin
    const char* test4[] = {"n", "n", "n", "n", "n", "n", "n", "n", "n", "n"};
    printf("Test 4: %s\\n", isValidWalk(test4, 10) ? "true" : "false");
    
    // Test case 5: Too long (more than 10 minutes)
    const char* test5[] = {"n", "s", "n", "s", "n", "s", "n", "s", "n", "s", "n", "s"};
    printf("Test 5: %s\\n", isValidWalk(test5, 12) ? "true" : "false");
    
    return 0;
}
