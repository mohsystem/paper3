#include <stdio.h>

int findIt(const int* arr, int size) {
    int xor_sum = 0;
    for (int i = 0; i < size; i++) {
        xor_sum ^= arr[i];
    }
    return xor_sum;
}

int main() {
    // Test cases
    int arr1[] = {7};
    printf("%d\n", findIt(arr1, sizeof(arr1) / sizeof(arr1[0]))); // Output: 7

    int arr2[] = {0};
    printf("%d\n", findIt(arr2, sizeof(arr2) / sizeof(arr2[0]))); // Output: 0

    int arr3[] = {1, 1, 2};
    printf("%d\n", findIt(arr3, sizeof(arr3) / sizeof(arr3[0]))); // Output: 2

    int arr4[] = {0, 1, 0, 1, 0};
    printf("%d\n", findIt(arr4, sizeof(arr4) / sizeof(arr4[0]))); // Output: 0

    int arr5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    printf("%d\n", findIt(arr5, sizeof(arr5) / sizeof(arr5[0]))); // Output: 4
    
    return 0;
}