#include <stdio.h>

int findMissingNumber(const int* arr, int len) {
    int n = len + 1;
    int xr = 0;
    for (int i = 1; i <= n; i++) xr ^= i;
    for (int i = 0; i < len; i++) xr ^= arr[i];
    return xr;
}

int main() {
    int arr1[] = {2, 3, 1, 5};
    int arr2[] = {1};
    int arr3[] = {2};
    int arr4[] = {1, 2, 3, 4, 5, 6, 7, 8, 10};
    int* arr5 = NULL; /* empty array */
    int len1 = sizeof(arr1) / sizeof(arr1[0]);
    int len2 = sizeof(arr2) / sizeof(arr2[0]);
    int len3 = sizeof(arr3) / sizeof(arr3[0]);
    int len4 = sizeof(arr4) / sizeof(arr4[0]);
    int len5 = 0;

    printf("Test 1 missing number: %d\n", findMissingNumber(arr1, len1));
    printf("Test 2 missing number: %d\n", findMissingNumber(arr2, len2));
    printf("Test 3 missing number: %d\n", findMissingNumber(arr3, len3));
    printf("Test 4 missing number: %d\n", findMissingNumber(arr4, len4));
    printf("Test 5 missing number: %d\n", findMissingNumber(arr5, len5));
    return 0;
}