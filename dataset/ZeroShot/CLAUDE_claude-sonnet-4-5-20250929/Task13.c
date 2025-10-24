
#include <stdio.h>
#include <stdbool.h>

bool isValidWalk(char walk[], int length) {
    if (length != 10) {
        return false;
    }
    
    int x = 0, y = 0;
    
    for (int i = 0; i < length; i++) {
        switch (walk[i]) {
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
    
    return x == 0 && y == 0;
}

int main() {
    // Test case 1: Valid walk
    char test1[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    printf("Test 1: %s\\n", isValidWalk(test1, 10) ? "true" : "false");
    
    // Test case 2: Valid walk with all directions
    char test2[] = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
    printf("Test 2: %s\\n", isValidWalk(test2, 10) ? "true" : "false");
    
    // Test case 3: Too short
    char test3[] = {'n', 's', 'n', 's', 'n'};
    printf("Test 3: %s\\n", isValidWalk(test3, 5) ? "true" : "false");
    
    // Test case 4: Correct length but doesn't return to start\n    char test4[] = {'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n', 'n'};\n    printf("Test 4: %s\\n", isValidWalk(test4, 10) ? "true" : "false");\n    \n    // Test case 5: Too long\n    char test5[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    printf("Test 5: %s\\n", isValidWalk(test5, 12) ? "true" : "false");
    
    return 0;
}
