#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int* data;
    int size;
} ArrayResult;

ArrayResult removeSmallest(const int* arr, int n) {
    ArrayResult res;
    if (arr == NULL || n <= 0) {
        res.data = NULL;
        res.size = 0;
        return res;
    }
    int minVal = arr[0];
    int minIdx = 0;
    for (int i = 1; i < n; ++i) {
        if (arr[i] < minVal) {
            minVal = arr[i];
            minIdx = i;
        }
    }
    if (n - 1 <= 0) {
        res.data = NULL;
        res.size = 0;
        return res;
    }
    res.data = (int*)malloc(sizeof(int) * (n - 1));
    res.size = n - 1;
    int j = 0;
    for (int i = 0; i < n; ++i) {
        if (i == minIdx) continue;
        res.data[j++] = arr[i];
    }
    return res;
}

void printArray(const int* a, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(",");
        printf("%d", a[i]);
    }
    printf("]\n");
}

int main() {
    int t1[] = {1,2,3,4,5};
    int t2[] = {5,3,2,1,4};
    int t3[] = {2,2,1,2,1};
    int* t4 = NULL; int n4 = 0;
    int t5[] = {10};

    ArrayResult r1 = removeSmallest(t1, 5);
    ArrayResult r2 = removeSmallest(t2, 5);
    ArrayResult r3 = removeSmallest(t3, 5);
    ArrayResult r4 = removeSmallest(t4, n4);
    ArrayResult r5 = removeSmallest(t5, 1);

    printArray(r1.data, r1.size);
    printArray(r2.data, r2.size);
    printArray(r3.data, r3.size);
    printArray(r4.data, r4.size);
    printArray(r5.data, r5.size);

    free(r1.data);
    free(r2.data);
    free(r3.data);
    free(r4.data);
    free(r5.data);
    return 0;
}