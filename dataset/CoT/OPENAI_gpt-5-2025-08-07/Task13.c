/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Validate that a sequence of steps (chars 'n','s','e','w') is length 10 and returns to origin.
 * 2) Security requirements: Validate inputs (non-null, correct length, allowed chars).
 * 3) Secure coding generation: Use const where applicable and avoid undefined behavior.
 * 4) Code review: Handle invalid chars and maintain simple, clear logic.
 * 5) Secure code output: Final code includes tests without external input.
 */

#include <stdio.h>
#include <stddef.h>

int isValidWalk(const char walk[], size_t len) {
    if (walk == NULL) return 0;
    if (len != 10) return 0;

    int x = 0, y = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = walk[i];
        switch (c) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            default: return 0; // invalid direction
        }
    }
    return (x == 0 && y == 0) ? 1 : 0;
}

int main(void) {
    // 5 test cases
    const char t1[10] = {'n','s','n','s','n','s','n','s','n','s'}; // true
    const char t2[5]  = {'w','e','w','e','w'};                     // false (length != 10)
    const char t3[10] = {'n','n','n','s','n','s','n','s','n','s'}; // false (doesn't return)
    const char t4[10] = {'e','w','e','w','e','w','e','w','e','w'}; // true
    const char t5[10] = {'n','n','n','n','n','s','s','s','s','e'}; // false (net east)

    printf("Test 1: %s\n", isValidWalk(t1, 10) ? "true" : "false");
    printf("Test 2: %s\n", isValidWalk(t2, 5)  ? "true" : "false");
    printf("Test 3: %s\n", isValidWalk(t3, 10) ? "true" : "false");
    printf("Test 4: %s\n", isValidWalk(t4, 10) ? "true" : "false");
    printf("Test 5: %s\n", isValidWalk(t5, 10) ? "true" : "false");

    return 0;
}