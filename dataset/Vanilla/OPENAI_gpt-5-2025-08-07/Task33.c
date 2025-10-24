#include <stdio.h>
#include <stdlib.h>

int queueTime(const int* customers, int len, int n) {
    if (n <= 0) return 0;
    int* tills = (int*)calloc(n, sizeof(int));
    if (!tills) return 0;

    for (int i = 0; i < len; ++i) {
        int minIdx = 0;
        for (int j = 1; j < n; ++j) {
            if (tills[j] < tills[minIdx]) minIdx = j;
        }
        tills[minIdx] += customers[i];
    }

    int total = 0;
    for (int i = 0; i < n; ++i) {
        if (tills[i] > total) total = tills[i];
    }
    free(tills);
    return total;
}

int main() {
    int a1[] = {5, 3, 4};
    int a2[] = {10, 2, 3, 3};
    int a3[] = {2, 3, 10};
    int a4[] = {};
    int a5[] = {1, 2, 3, 4, 5};

    printf("%d\n", queueTime(a1, sizeof(a1)/sizeof(a1[0]), 1));
    printf("%d\n", queueTime(a2, sizeof(a2)/sizeof(a2[0]), 2));
    printf("%d\n", queueTime(a3, sizeof(a3)/sizeof(a3[0]), 2));
    printf("%d\n", queueTime(a4, 0, 1));
    printf("%d\n", queueTime(a5, sizeof(a5)/sizeof(a5[0]), 100));

    return 0;
}