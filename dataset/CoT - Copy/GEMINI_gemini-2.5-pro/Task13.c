#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// In C, we need to pass the size of the array explicitly.
bool isValidWalk(const char walk[], size_t size) {
    // 1. The walk must take exactly 10 minutes.
    if (size != 10) {
        return false;
    }

    // 2. The walk must return you to your starting point.
    int x = 0; // Represents East-West axis
    int y = 0; // Represents North-South axis

    for (size_t i = 0; i < size; ++i) {
        switch (walk[i]) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
        }
    }

    return x == 0 && y == 0;
}

void runTest(const char* testName, const char walk[], size_t size) {
    printf("%s: {", testName);
    for (size_t i = 0; i < size; ++i) {
        printf("'%c'%s", walk[i], (i == size - 1) ? "" : ", ");
    }
    printf("}\n");
    printf("Result: %s\n\n", isValidWalk(walk, size) ? "true" : "false");
}

int main() {
    // Test Cases
    char test1[] = {'n','s','n','s','n','s','n','s','n','s'};
    runTest("Test Case 1", test1, sizeof(test1)/sizeof(test1[0]));

    char test2[] = {'w','e','w','e','w','e','w','e','w','e'};
    runTest("Test Case 2", test2, sizeof(test2)/sizeof(test2[0]));

    char test3[] = {'w'};
    runTest("Test Case 3", test3, sizeof(test3)/sizeof(test3[0]));

    char test4[] = {'n','n','n','s','n','s','n','s','n','s'};
    runTest("Test Case 4", test4, sizeof(test4)/sizeof(test4[0]));
    
    char test5[] = {'e','e','e','e','w','w','w','w','w','w'};
    runTest("Test Case 5", test5, sizeof(test5)/sizeof(test5[0]));

    return 0;
}