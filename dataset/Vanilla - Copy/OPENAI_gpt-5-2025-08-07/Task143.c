#include <stdio.h>
#include <stdlib.h>

int cmpInt(const void* a, const void* b) {
    int ia = *(const int*)a;
    int ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

int* sortArray(int* arr, int n) {
    if (arr != NULL && n > 1) {
        qsort(arr, n, sizeof(int), cmpInt);
    }
    return arr;
}

void printArray(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        printf("%d", arr[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int main() {
    int a1[] = {3, 1, 2};
    int n1 = sizeof(a1) / sizeof(a1[0]);
    sortArray(a1, n1);
    printArray(a1, n1);

    int a2[] = {5, -1, 4, 4, 2};
    int n2 = sizeof(a2) / sizeof(a2[0]);
    sortArray(a2, n2);
    printArray(a2, n2);

    int* a3 = NULL;
    int n3 = 0;
    sortArray(a3, n3);
    printArray(a3, n3);

    int a4[] = {10};
    int n4 = sizeof(a4) / sizeof(a4[0]);
    sortArray(a4, n4);
    printArray(a4, n4);

    int a5[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    int n5 = sizeof(a5) / sizeof(a5[0]);
    sortArray(a5, n5);
    printArray(a5, n5);

    return 0;
}