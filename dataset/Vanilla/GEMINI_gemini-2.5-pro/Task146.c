#include <stdio.h>

int findMissingNumber(int arr[], int size) {
    // The array is supposed to have numbers from 1 to n, but has n-1 elements.
    // So, n is the size of the array + 1.
    long long n = size + 1;

    // Calculate the expected sum of the first n natural numbers.
    long long expectedSum = n * (n + 1) / 2;

    // Calculate the actual sum of elements in the array.
    long long actualSum = 0;
    for (int i = 0; i < size; i++) {
        actualSum += arr[i];
    }

    // The difference is the missing number.
    return (int)(expectedSum - actualSum);
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 4, 5};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Test Case 1: Missing number in [1, 2, 4, 5] is %d\n", findMissingNumber(arr1, size1));

    // Test Case 2
    int arr2[] = {2, 3, 1, 5};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Test Case 2: Missing number in [2, 3, 1, 5] is %d\n", findMissingNumber(arr2, size2));

    // Test Case 3
    int arr3[] = {1};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Test Case 3: Missing number in [1] is %d\n", findMissingNumber(arr3, size3));

    // Test Case 4
    int arr4[] = {1, 2, 3, 4, 5, 6, 8, 9};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Test Case 4: Missing number in [1, 2, 3, 4, 5, 6, 8, 9] is %d\n", findMissingNumber(arr4, size4));
    
    // Test Case 5
    int arr5[] = {2};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Test Case 5: Missing number in [2] is %d\n", findMissingNumber(arr5, size5));

    return 0;
}