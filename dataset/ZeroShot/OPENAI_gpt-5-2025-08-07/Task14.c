#include <stdio.h>
#include <stddef.h>

int find_even_index(const int *arr, size_t n) {
    if (arr == NULL || n == 0) return -1;
    long long total = 0;
    for (size_t i = 0; i < n; ++i) {
        total += (long long)arr[i];
    }
    long long left = 0;
    for (size_t i = 0; i < n; ++i) {
        long long right = total - left - (long long)arr[i];
        if (left == right) return (int)i;
        left += (long long)arr[i];
    }
    return -1;
}

int main(void) {
    int arr1[] = {1,2,3,4,3,2,1};           // expect 3
    int arr2[] = {1,100,50,-51,1,1};        // expect 1
    int arr3[] = {20,10,-80,10,10,15,35};   // expect 0
    int arr4[] = {1,2,3,4,5};               // expect -1
    int arr5[] = {0,0,0};                   // expect 0

    printf("Test 1 -> %d\n", find_even_index(arr1, sizeof(arr1)/sizeof(arr1[0])));
    printf("Test 2 -> %d\n", find_even_index(arr2, sizeof(arr2)/sizeof(arr2[0])));
    printf("Test 3 -> %d\n", find_even_index(arr3, sizeof(arr3)/sizeof(arr3[0])));
    printf("Test 4 -> %d\n", find_even_index(arr4, sizeof(arr4)/sizeof(arr4[0])));
    printf("Test 5 -> %d\n", find_even_index(arr5, sizeof(arr5)/sizeof(arr5[0])));
    return 0;
}