#include <stdio.h>

/* Step 1-5: Secure, bounds-checked retrieval from a fixed array. */
int getValueAtIndex(int index) {
    static const int DATA[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    const int n = (int)(sizeof(DATA) / sizeof(DATA[0]));
    if (index < 0 || index >= n) {
        return -1; /* out-of-bounds indicator */
    }
    return DATA[index];
}

int main(void) {
    int testCases[5] = {5, 0, 9, -1, 10};
    for (int i = 0; i < 5; ++i) {
        int idx = testCases[i];
        int result = getValueAtIndex(idx);
        printf("Index %d -> %d\n", idx, result);
    }
    return 0;
}