#include <stdio.h>
#include <stddef.h>

int isValidWalk(const char* walk, size_t len) {
    if (walk == NULL) {
        return 0;
    }

    // Validate length: must be exactly 10 minutes/steps
    if (len != 10U) {
        return 0;
    }

    int x = 0;
    int y = 0;

    for (size_t i = 0; i < len; ++i) {
        char dir = walk[i];
        switch (dir) {
            case 'n': y += 1; break;
            case 's': y -= 1; break;
            case 'e': x += 1; break;
            case 'w': x -= 1; break;
            default:  // Invalid direction
                return 0;
        }
    }

    return (x == 0 && y == 0) ? 1 : 0;
}

static const char* b2s(int v) {
    return v ? "true" : "false";
}

int main(void) {
    // Test case 1: Valid 10-minute walk returning to start
    const char t1[] = {'n','s','n','s','n','s','n','s','n','s'};
    printf("Test 1: %s\n", b2s(isValidWalk(t1, sizeof(t1)/sizeof(t1[0]))));

    // Test case 2: Less than 10 minutes
    const char t2[] = {'n','s','n','s','n','s','n','s'};
    printf("Test 2: %s\n", b2s(isValidWalk(t2, sizeof(t2)/sizeof(t2[0]))));

    // Test case 3: 10 minutes but does not return to start
    const char t3[] = {'n','n','n','n','n','s','s','s','s','e'};
    printf("Test 3: %s\n", b2s(isValidWalk(t3, sizeof(t3)/sizeof(t3[0]))));

    // Test case 4: Contains invalid character
    const char t4[] = {'n','s','x','s','n','s','n','s','n','s'};
    printf("Test 4: %s\n", b2s(isValidWalk(t4, sizeof(t4)/sizeof(t4[0]))));

    // Test case 5: Another valid 10-minute walk returning to start
    const char t5[] = {'e','w','e','w','n','s','n','s','e','w'};
    printf("Test 5: %s\n", b2s(isValidWalk(t5, sizeof(t5)/sizeof(t5[0]))));

    return 0;
}