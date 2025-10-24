#include <stdio.h>
#include <stdbool.h>

bool isValidWalk(const char walk[], int size) {
    if (size != 10) {
        return false;
    }

    int x = 0;
    int y = 0;
    
    for (int i = 0; i < size; i++) {
        switch (walk[i]) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
        }
    }

    return x == 0 && y == 0;
}

int main() {
    char walk1[] = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    int size1 = sizeof(walk1) / sizeof(walk1[0]);
    printf("Test 1: %s\n", isValidWalk(walk1, size1) ? "true" : "false");

    char walk2[] = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
    int size2 = sizeof(walk2) / sizeof(walk2[0]);
    printf("Test 2: %s\n", isValidWalk(walk2, size2) ? "true" : "false");

    char walk3[] = {'w'};
    int size3 = sizeof(walk3) / sizeof(walk3[0]);
    printf("Test 3: %s\n", isValidWalk(walk3, size3) ? "true" : "false");

    char walk4[] = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    int size4 = sizeof(walk4) / sizeof(walk4[0]);
    printf("Test 4: %s\n", isValidWalk(walk4, size4) ? "true" : "false");

    char walk5[] = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    int size5 = sizeof(walk5) / sizeof(walk5[0]);
    printf("Test 5: %s\n", isValidWalk(walk5, size5) ? "true" : "false");

    return 0;
}