#include <stdio.h>
#include <stdlib.h>

int longestRun(const int* arr, int n) {
    if (arr == NULL || n <= 0) return 0;
    int maxLen = 1, curLen = 1, dir = 0; // 1 inc, -1 dec, 0 unknown
    for (int i = 1; i < n; i++) {
        int diff = arr[i] - arr[i - 1];
        if (diff == 1) {
            if (dir == 1 || dir == 0) curLen++;
            else curLen = 2;
            dir = 1;
        } else if (diff == -1) {
            if (dir == -1 || dir == 0) curLen++;
            else curLen = 2;
            dir = -1;
        } else {
            curLen = 1;
            dir = 0;
        }
        if (curLen > maxLen) maxLen = curLen;
    }
    return maxLen;
}

static void printResult(const int* arr, int n) {
    printf("%d\n", longestRun(arr, n));
}

int main(void) {
    int a1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    int a2[] = {1, 2, 3, 10, 11, 15};
    int a3[] = {5, 4, 2, 1};
    int a4[] = {3, 5, 7, 10, 15};
    int a5[] = {7};
    printResult(a1, sizeof(a1)/sizeof(a1[0])); // 5
    printResult(a2, sizeof(a2)/sizeof(a2[0])); // 3
    printResult(a3, sizeof(a3)/sizeof(a3[0])); // 2
    printResult(a4, sizeof(a4)/sizeof(a4[0])); // 1
    printResult(a5, sizeof(a5)/sizeof(a5[0])); // 1
    return 0;
}