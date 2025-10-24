#include <stdio.h>

double findUniq(const double arr[], int size) {
    // It's guaranteed that array contains at least 3 numbers.
    // If the first two numbers are different, one of them is the unique number.
    // We can check the third number to decide which one.
    if (arr[0] != arr[1]) {
        // If arr[0] is equal to arr[2], then arr[1] must be the unique number.
        // Otherwise, arr[0] is the unique number.
        return arr[0] == arr[2] ? arr[1] : arr[0];
    } else {
        // If the first two numbers are the same, this is the common number.
        // We can then iterate through the rest of the array to find the different one.
        double commonNumber = arr[0];
        for (int i = 2; i < size; ++i) {
            if (arr[i] != commonNumber) {
                return arr[i];
            }
        }
    }
    // This part should be unreachable given the problem constraints.
    return -1.0;
}

int main() {
    // Test Case 1
    double arr1[] = {1, 1, 1, 2, 1, 1};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("%f\n", findUniq(arr1, size1)); // Expected: 2.000000

    // Test Case 2
    double arr2[] = {0, 0, 0.55, 0, 0};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("%f\n", findUniq(arr2, size2)); // Expected: 0.550000

    // Test Case 3
    double arr3[] = {3, 10, 3, 3, 3};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("%f\n", findUniq(arr3, size3)); // Expected: 10.000000

    // Test Case 4
    double arr4[] = {5, 5, 5, 5, 4};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("%f\n", findUniq(arr4, size4)); // Expected: 4.000000

    // Test Case 5
    double arr5[] = {8, 8, 8, 8, 8, 8, 8, 7};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("%f\n", findUniq(arr5, size5)); // Expected: 7.000000

    return 0;
}