#include <stdio.h>

int findOdd(const int arr[], int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= arr[i];
    }
    return result;
}

void printArray(const int arr[], int size) {
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]");
}

int main() {
    // Test Case 1
    int arr1[] = {7};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1: ");
    printArray(arr1, size1);
    printf(" -> %d\n", findOdd(arr1, size1));

    // Test Case 2
    int arr2[] = {0};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2: ");
    printArray(arr2, size2);
    printf(" -> %d\n", findOdd(arr2, size2));

    // Test Case 3
    int arr3[] = {1, 1, 2};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3: ");
    printArray(arr3, size3);
    printf(" -> %d\n", findOdd(arr3, size3));

    // Test Case 4
    int arr4[] = {0, 1, 0, 1, 0};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4: ");
    printArray(arr4, size4);
    printf(" -> %d\n", findOdd(arr4, size4));

    // Test Case 5
    int arr5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5: ");
    printArray(arr5, size5);
    printf(" -> %d\n", findOdd(arr5, size5));

    return 0;
}