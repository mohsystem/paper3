#include <stdio.h>
#include <stdlib.h>

int cmp_int(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

int* posNegSort(const int* arr, int n) {
    if (n <= 0 || arr == NULL) return NULL;
    int* res = (int*)malloc(sizeof(int) * n);
    if (!res) return NULL;

    int posCount = 0;
    for (int i = 0; i < n; ++i) if (arr[i] > 0) posCount++;

    int* pos = (int*)malloc(sizeof(int) * (posCount > 0 ? posCount : 1));
    if (!pos) { free(res); return NULL; }

    int idx = 0;
    for (int i = 0; i < n; ++i) if (arr[i] > 0) pos[idx++] = arr[i];

    qsort(pos, posCount, sizeof(int), cmp_int);

    int p = 0;
    for (int i = 0; i < n; ++i) {
        if (arr[i] > 0) res[i] = pos[p++];
        else res[i] = arr[i];
    }

    free(pos);
    return res;
}

void printArray(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    int arr1[] = {6, 3, -2, 5, -8, 2, -2};
    int n1 = sizeof(arr1) / sizeof(arr1[0]);
    int arr2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int n2 = sizeof(arr2) / sizeof(arr2[0]);
    int arr3[] = {-5, -5, -5, -5, 7, -5};
    int n3 = sizeof(arr3) / sizeof(arr3[0]);
    int* arr4 = NULL; int n4 = 0;
    int arr5[] = {9, -3, 8, -1, -2, 7, 6};
    int n5 = sizeof(arr5) / sizeof(arr5[0]);

    int* r1 = posNegSort(arr1, n1);
    int* r2 = posNegSort(arr2, n2);
    int* r3 = posNegSort(arr3, n3);
    int* r4 = posNegSort(arr4, n4);
    int* r5 = posNegSort(arr5, n5);

    printArray(r1, n1);
    printArray(r2, n2);
    printArray(r3, n3);
    printArray(r4, n4);
    printArray(r5, n5);

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);

    return 0;
}