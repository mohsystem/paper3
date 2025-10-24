#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * Checks if a given walk is valid.
 * A valid walk takes exactly 10 minutes (10 steps) and returns to the starting point.
 *
 * @param walk A pointer to an array of characters representing directions.
 * @param size The number of elements in the walk array.
 * @return true if the walk is valid, false otherwise.
 */
bool isValidWalk(const char* walk, size_t size) {
    // The walk must be exactly 10 minutes long.
    if (walk == NULL || size != 10) {
        return false;
    }

    int x = 0; // Represents horizontal displacement (East/West)
    int y = 0; // Represents vertical displacement (North/South)

    for (size_t i = 0; i < size; i++) {
        switch (walk[i]) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            // According to the prompt, we don't need to handle other cases.
        }
    }

    // The walk must return to the starting point (0,0).
    return x == 0 && y == 0;
}

int main() {
    // Test Case 1: Valid walk
    char walk1[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    size_t size1 = sizeof(walk1) / sizeof(walk1[0]);
    printf("Test Case 1: %s\n", isValidWalk(walk1, size1) ? "true" : "false"); // Expected: true

    // Test Case 2: Too long walk
    char walk2[] = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
    size_t size2 = sizeof(walk2) / sizeof(walk2[0]);
    printf("Test Case 2: %s\n", isValidWalk(walk2, size2) ? "true" : "false"); // Expected: false

    // Test Case 3: Too short walk
    char walk3[] = {'w'};
    size_t size3 = sizeof(walk3) / sizeof(walk3[0]);
    printf("Test Case 3: %s\n", isValidWalk(walk3, size3) ? "true" : "false"); // Expected: false

    // Test Case 4: Correct length, but does not return to start
    char walk4[] = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    size_t size4 = sizeof(walk4) / sizeof(walk4[0]);
    printf("Test Case 4: %s\n", isValidWalk(walk4, size4) ? "true" : "false"); // Expected: false

    // Test Case 5: Valid walk with mixed directions
    char walk5[] = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    size_t size5 = sizeof(walk5) / sizeof(walk5[0]);
    printf("Test Case 5: %s\n", isValidWalk(walk5, size5) ? "true" : "false"); // Expected: true

    return 0;
}