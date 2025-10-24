#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int binary_search(const int* arr, int n, int target) {
    int lo = 0, hi = n - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        int val = arr[mid];
        if (val == target) return mid;
        if (val < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

int main(void) {
    clock_t start = clock();

    int arr1[] = {1, 3, 5, 7, 9};
    int arr2[] = {1, 3, 5, 7, 9};
    int arr3[] = {};
    int arr4[] = {5};
    int arr5[] = {-10, -3, 0, 5, 5, 9, 12};

    int idx1 = binary_search(arr1, 5, 7);
    int idx2 = binary_search(arr2, 5, 2);
    int idx3 = binary_search(arr3, 0, 1);
    int idx4 = binary_search(arr4, 1, 5);
    int idx5 = binary_search(arr5, 7, 5);

    printf("Test1 index: %d\n", idx1);
    printf("Test2 index: %d\n", idx2);
    printf("Test3 index: %d\n", idx3);
    printf("Test4 index: %d\n", idx4);
    printf("Test5 index: %d\n", idx5);

    clock_t end = clock();
    double elapsed_seconds = (double)(end - start) / CLOCKS_PER_SEC;

    char tmpname[L_tmpnam];
    if (tmpnam(tmpname) == NULL) {
        fprintf(stderr, "Failed to create temp filename\n");
        return 1;
    }

    FILE* f = fopen(tmpname, "w");
    if (!f) {
        fprintf(stderr, "Failed to open temp file for writing\n");
        return 1;
    }
    fprintf(f, "elapsed_seconds=%f\nelapsed_millis=%f\n", elapsed_seconds, elapsed_seconds * 1000.0);
    fclose(f);

    printf("Wrote timing to: %s\n", tmpname);
    return 0;
}