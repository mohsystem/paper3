#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* data;
    int size;
} IntArray;

IntArray mergeSortedArrays(const int* a, int n, const int* b, int m) {
    IntArray result;
    result.size = n + m;
    result.data = (int*)malloc(sizeof(int) * result.size);
    if (!result.data) {
        result.size = 0;
        return result;
    }
    int i = 0, j = 0, k = 0;
    while (i < n && j < m) {
        if (a[i] <= b[j]) {
            result.data[k++] = a[i++];
        } else {
            result.data[k++] = b[j++];
        }
    }
    while (i < n) result.data[k++] = a[i++];
    while (j < m) result.data[k++] = b[j++];
    return result;
}

void printIntArray(const IntArray arr) {
    printf("[");
    for (int i = 0; i < arr.size; ++i) {
        if (i) printf(", ");
        printf("%d", arr.data[i]);
    }
    printf("]\n");
}

int main() {
    int A1[] = {1, 3, 5}; int nA1 = 3;
    int B1[] = {2, 4, 6}; int nB1 = 3;

    int* A2 = NULL; int nA2 = 0;
    int B2[] = {1, 2, 3}; int nB2 = 3;

    int A3[] = {1, 1, 2, 2}; int nA3 = 4;
    int B3[] = {1, 2, 2, 3}; int nB3 = 4;

    int A4[] = {-5, -3, 0, 2}; int nA4 = 4;
    int B4[] = {-4, -1, 3}; int nB4 = 3;

    int A5[] = {1, 2, 7, 8, 9}; int nA5 = 5;
    int B5[] = {3, 4}; int nB5 = 2;

    IntArray R1 = mergeSortedArrays(A1, nA1, B1, nB1);
    IntArray R2 = mergeSortedArrays(A2, nA2, B2, nB2);
    IntArray R3 = mergeSortedArrays(A3, nA3, B3, nB3);
    IntArray R4 = mergeSortedArrays(A4, nA4, B4, nB4);
    IntArray R5 = mergeSortedArrays(A5, nA5, B5, nB5);

    printIntArray(R1);
    printIntArray(R2);
    printIntArray(R3);
    printIntArray(R4);
    printIntArray(R5);

    free(R1.data);
    free(R2.data);
    free(R3.data);
    free(R4.data);
    free(R5.data);

    return 0;
}