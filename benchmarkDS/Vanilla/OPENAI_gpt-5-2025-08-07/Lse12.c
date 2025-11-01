#include <stdio.h>
#include <stdlib.h>

int getValueFromArray(const int* arr, int n, int idx) {
    return arr[idx];
}

int main(void) {
    int idx;
    if (scanf("%d", &idx) != 1) {
        idx = 0;
    }

    int a1[] = {10, 20, 30, 40, 50};
    int a2[] = {1, 2, 3, 4};
    int a3[] = {7};
    int a4[] = {100, 200};
    int a5[] = {5, 4, 3, 2, 1, 0};

    const int* arrays[] = {a1, a2, a3, a4, a5};
    int sizes[] = {5, 4, 1, 2, 6};

    int testIdx = abs(idx) % 5;
    const int* selected = arrays[testIdx];
    int n = sizes[testIdx];
    int safeIndex = idx % n;
    if (safeIndex < 0) safeIndex += n;

    int result = getValueFromArray(selected, n, safeIndex);
    return result;
}