#include <stdio.h>

int xorGame(const int* nums, int n) {
    int xr = 0;
    for (int i = 0; i < n; ++i) xr ^= nums[i];
    if (xr == 0) return 1;
    return (n % 2 == 0) ? 1 : 0;
}

int main() {
    int t1[] = {1, 1, 2};
    int t2[] = {0, 1};
    int t3[] = {1, 2, 3};
    int t4[] = {2};
    int t5[] = {1, 1};

    printf("%s\n", xorGame(t1, 3) ? "true" : "false");
    printf("%s\n", xorGame(t2, 2) ? "true" : "false");
    printf("%s\n", xorGame(t3, 3) ? "true" : "false");
    printf("%s\n", xorGame(t4, 1) ? "true" : "false");
    printf("%s\n", xorGame(t5, 2) ? "true" : "false");
    return 0;
}