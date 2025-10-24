#include <stdio.h>

int longestRun(const int* arr, int n) {
    if (arr == NULL || n <= 0) return 0;
    int maxLen = 1;
    int currLen = 1;
    int prevDiffSet = 0; // 0: none, 1: set
    int prevDiff = 0;    // +1 or -1

    for (int i = 1; i < n; i++) {
        int diff = arr[i] - arr[i - 1];
        if (diff == 1 || diff == -1) {
            if (prevDiffSet && diff == prevDiff) {
                currLen++;
            } else {
                currLen = 2;
                prevDiff = diff;
                prevDiffSet = 1;
            }
        } else {
            currLen = 1;
            prevDiffSet = 0;
        }
        if (currLen > maxLen) maxLen = currLen;
    }
    return maxLen;
}

int main() {
    int a1[] = {1, 2, 3, 5, 6, 7, 8, 9};        // 5
    int a2[] = {1, 2, 3, 10, 11, 15};           // 3
    int a3[] = {5, 4, 2, 1};                    // 2
    int a4[] = {3, 5, 7, 10, 15};               // 1
    int a5[] = {1, 2, 1, 0, -1};                // 4

    printf("%d\n", longestRun(a1, sizeof(a1)/sizeof(a1[0])));
    printf("%d\n", longestRun(a2, sizeof(a2)/sizeof(a2[0])));
    printf("%d\n", longestRun(a3, sizeof(a3)/sizeof(a3[0])));
    printf("%d\n", longestRun(a4, sizeof(a4)/sizeof(a4[0])));
    printf("%d\n", longestRun(a5, sizeof(a5)/sizeof(a5[0])));
    return 0;
}