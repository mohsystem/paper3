#include <stdio.h>

int findOdd(const int* arr, int n) {
    int x = 0;
    for (int i = 0; i < n; i++) x ^= arr[i];
    return x;
}

int main() {
    int t1[] = {7};
    int t2[] = {0};
    int t3[] = {1, 1, 2};
    int t4[] = {0, 1, 0, 1, 0};
    int t5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};

    printf("%d\n", findOdd(t1, sizeof(t1)/sizeof(t1[0])));
    printf("%d\n", findOdd(t2, sizeof(t2)/sizeof(t2[0])));
    printf("%d\n", findOdd(t3, sizeof(t3)/sizeof(t3[0])));
    printf("%d\n", findOdd(t4, sizeof(t4)/sizeof(t4[0])));
    printf("%d\n", findOdd(t5, sizeof(t5)/sizeof(t5[0])));

    return 0;
}