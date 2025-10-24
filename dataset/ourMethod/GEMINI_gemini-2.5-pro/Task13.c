#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Checks if a given walk is valid.
 * A valid walk takes exactly 10 minutes (10 steps) and returns to the starting point.
 * @param walk An array of characters representing the directions of the walk ('n', 's', 'e', 'w').
 * @param len The number of elements in the walk array.
 * @return true if the walk is valid, false otherwise.
 */
bool isValidWalk(const char walk[], size_t len) {
    // A valid walk must be exactly 10 minutes long.
    if (len != 10) {
        return false;
    }

    int x = 0; // Represents horizontal position (East/West)
    int y = 0; // Represents vertical position (North/South)

    // Iterate through each step of the walk and update coordinates.
    for (size_t i = 0; i < len; i++) {
        switch (walk[i]) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            // Per problem description, only valid characters will be in the input.
        }
    }

    // The walk is valid if it returns to the origin (0,0).
    return x == 0 && y == 0;
}

int main() {
    // Test Case 1: Valid walk
    char walk1[] = {'n','s','n','s','n','s','n','s','n','s'};
    size_t len1 = sizeof(walk1) / sizeof(walk1[0]);
    printf("Test 1: {'n','s','n','s','n','s','n','s','n','s'} -> %s\n", 
           isValidWalk(walk1, len1) ? "true" : "false");

    // Test Case 2: Invalid walk (too long)
    char walk2[] = {'w','e','w','e','w','e','w','e','w','e','w','e'};
    size_t len2 = sizeof(walk2) / sizeof(walk2[0]);
    printf("Test 2: {'w','e','w','e','w','e','w','e','w','e','w','e'} -> %s\n", 
           isValidWalk(walk2, len2) ? "true" : "false");

    // Test Case 3: Invalid walk (too short)
    char walk3[] = {'w'};
    size_t len3 = sizeof(walk3) / sizeof(walk3[0]);
    printf("Test 3: {'w'} -> %s\n", 
           isValidWalk(walk3, len3) ? "true" : "false");

    // Test Case 4: Invalid walk (10 minutes but doesn't return to start)
    char walk4[] = {'n','n','n','s','n','s','n','s','n','s'};
    size_t len4 = sizeof(walk4) / sizeof(walk4[0]);
    printf("Test 4: {'n','n','n','s','n','s','n','s','n','s'} -> %s\n", 
           isValidWalk(walk4, len4) ? "true" : "false");

    // Test Case 5: Valid walk
    char walk5[] = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    size_t len5 = sizeof(walk5) / sizeof(walk5[0]);
    printf("Test 5: {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'} -> %s\n", 
           isValidWalk(walk5, len5) ? "true" : "false");

    return 0;
}