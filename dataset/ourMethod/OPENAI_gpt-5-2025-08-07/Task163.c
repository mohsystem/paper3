#include <stdio.h>
#include <stddef.h>

int longestRun(const int* arr, size_t n) {
    if (arr == NULL && n == 0) return 0;
    if (n == 0) return 0;
    if (n == 1) return 1;

    int best = 1;
    int curr = 1;
    long long dir = 0; /* 0 none, +1 inc, -1 dec */

    for (size_t i = 1; i < n; ++i) {
        long long diff = (long long)arr[i] - (long long)arr[i - 1];
        if ((diff == 1 || diff == -1) && diff == dir) {
            curr += 1;
        } else if (diff == 1 || diff == -1) {
            curr = 2;
            dir = diff;
        } else {
            curr = 1;
            dir = 0;
        }
        if (curr > best) best = curr;
    }
    return best;
}

static void printArray(const int* arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        if (i > 0) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]");
}

int main(void) {
    int a1[] = {1, 2, 3, 5, 6, 7, 8, 9};
    int a2[] = {1, 2, 3, 10, 11, 15};
    int a3[] = {5, 4, 2, 1};
    int a4[] = {3, 5, 7, 10, 15};
    /* empty array -> pass NULL with length 0 */
    const int* a5 = NULL;
    size_t n1 = sizeof(a1) / sizeof(a1[0]);
    size_t n2 = sizeof(a2) / sizeof(a2[0]);
    size_t n3 = sizeof(a3) / sizeof(a3[0]);
    size_t n4 = sizeof(a4) / sizeof(a4[0]);
    size_t n5 = 0;

    printArray(a1, n1); printf(" -> %d\n", longestRun(a1, n1));
    printArray(a2, n2); printf(" -> %d\n", longestRun(a2, n2));
    printArray(a3, n3); printf(" -> %d\n", longestRun(a3, n3));
    printArray(a4, n4); printf(" -> %d\n", longestRun(a4, n4));
    printArray(a5, n5); printf(" -> %d\n", longestRun(a5, n5));

    return 0;
}