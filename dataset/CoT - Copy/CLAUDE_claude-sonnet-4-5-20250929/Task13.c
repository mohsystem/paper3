
#include <stdio.h>
#include <stdbool.h>

bool isValidWalk(const char walk[], int length) {
    // Check if walk is NULL to prevent segmentation fault
    if (walk == NULL) {
        return false;
    }
    
    // Check if the walk takes exactly 10 minutes
    if (length != 10) {
        return false;
    }
    
    // Track position on grid
    int x = 0;
    int y = 0;
    
    // Process each direction
    for (int i = 0; i < length; i++) {
        char direction = walk[i];
        
        // Validate that only allowed directions are present
        if (direction != 'n' && direction != 's' && 
            direction != 'e' && direction != 'w') {
            return false;
        }
        
        // Update position based on direction
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
        }
    }
    
    // Check if we returned to starting point
    return x == 0 && y == 0;
}

int main() {
    // Test case 1: Valid 10-minute walk returning to start
    char test1[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    printf("Test 1: %s\\n", isValidWalk(test1, 10) ? "true" : "false");
    
    // Test case 2: Valid 10-minute walk with all directions
    char test2[] = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
    printf("Test 2: %s\\n", isValidWalk(test2, 10) ? "true" : "false");
    
    // Test case 3: Too short walk
    char test3[] = {'n', 's', 'e', 'w'};
    printf("Test 3: %s\\n", isValidWalk(test3, 4) ? "true" : "false");
    
    // Test case 4: Correct length but doesn't return to start\n    char test4[] = {'n', 'n', 'n', 's', 's', 's', 'e', 'w', 'n', 's'};\n    printf("Test 4: %s\\n", isValidWalk(test4, 10) ? "true" : "false");\n    \n    // Test case 5: Too long walk\n    char test5[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    printf("Test 5: %s\\n", isValidWalk(test5, 12) ? "true" : "false");
    
    return 0;
}
