#include <stdio.h>
#include <stdbool.h>

bool xorGame(const int* nums, int n) {
    if (nums == NULL && n > 0) return false;
    int x = 0;
    for (int i = 0; i < n; ++i) x ^= nums[i];
    if (x == 0) return true;
    return (n % 2) == 0;
}

static const char* boolStr(bool b) { return b ? "true" : "false"; }

int main(void) {
    int a1[] = {1, 1, 2};   // false
    int a2[] = {0, 1};      // true
    int a3[] = {1, 2, 3};   // true
    int a4[] = {5};         // false
    int a5[] = {2, 2};      // true

    int* tests[] = {a1, a2, a3, a4, a5};
    int lens[] = {3, 2, 3, 1, 2};

    for (int t = 0; t < 5; ++t) {
        printf("Input: [");
        for (int i = 0; i < lens[t]; ++i) {
            if (i) printf(", ");
            printf("%d", tests[t][i]);
        }
        printf("] -> %s\n", boolStr(xorGame(tests[t], lens[t])));
    }
    return 0;
}