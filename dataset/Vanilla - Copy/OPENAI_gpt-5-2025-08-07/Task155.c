#include <stdio.h>
#include <stdlib.h>

static void heapify(int* arr, int heapSize, int i) {
    while (1) {
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < heapSize && arr[l] > arr[largest]) largest = l;
        if (r < heapSize && arr[r] > arr[largest]) largest = r;
        if (largest != i) {
            int t = arr[i];
            arr[i] = arr[largest];
            arr[largest] = t;
            i = largest;
        } else {
            break;
        }
    }
}

int* heapSort(int* arr, int n) {
    if (arr == NULL || n <= 1) return arr;
    for (int i = n / 2 - 1; i >= 0; --i) {
        heapify(arr, n, i);
    }
    for (int i = n - 1; i > 0; --i) {
        int t = arr[0];
        arr[0] = arr[i];
        arr[i] = t;
        heapify(arr, i, 0);
    }
    return arr;
}

static void print_array(const int* arr, int n) {
    printf("[");
    for (int i = 0; i < n; ++i) {
        if (i) printf(", ");
        printf("%d", arr[i]);
    }
    printf("]\n");
}

int main() {
    int arr1[] = {4, 10, 3, 5, 1};
    int* arr2 = NULL; int n2 = 0;
    int arr3[] = {1};
    int arr4[] = {9, -2, 0, 7, 3, 3, -5};
    int arr5[] = {5, 4, 3, 2, 1};

    int n1 = sizeof(arr1)/sizeof(arr1[0]);
    int n3 = sizeof(arr3)/sizeof(arr3[0]);
    int n4 = sizeof(arr4)/sizeof(arr4[0]);
    int n5 = sizeof(arr5)/sizeof(arr5[0]);

    heapSort(arr1, n1); print_array(arr1, n1);
    heapSort(arr2, n2); print_array(arr2, n2);
    heapSort(arr3, n3); print_array(arr3, n3);
    heapSort(arr4, n4); print_array(arr4, n4);
    heapSort(arr5, n5); print_array(arr5, n5);

    return 0;
}