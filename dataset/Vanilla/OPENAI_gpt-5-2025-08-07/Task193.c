#include <stdio.h>
#include <stdbool.h>

bool chalkboardXorGame(const int* nums, int n) {
    int xr = 0;
    for (int i = 0; i < n; ++i) xr ^= nums[i];
    if (xr == 0) return true;
    return (n % 2 == 0);
}

int main() {
    int t1[] = {1, 1, 2};
    int t2[] = {0, 1};
    int t3[] = {1, 2, 3};
    int t4[] = {0};
    int t5[] = {5};

    printf("%s\n", chalkboardXorGame(t1, 3) ? "true" : "false");
    printf("%s\n", chalkboardXorGame(t2, 2) ? "true" : "false");
    printf("%s\n", chalkboardXorGame(t3, 3) ? "true" : "false");
    printf("%s\n", chalkboardXorGame(t4, 1) ? "true" : "false");
    printf("%s\n", chalkboardXorGame(t5, 1) ? "true" : "false");
    return 0;
}