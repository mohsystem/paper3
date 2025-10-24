#include <stdio.h>
#include <stddef.h>

double findUniq(const double arr[], size_t size) {
    // Since the array is guaranteed to have at least 3 elements,
    // we can check the first three to determine the unique number.
    if (arr[0] != arr[1]) {
        // If the first two are different, one of them is the unique number.
        // The third element will tell us which one is the common number.
        if (arr[0] == arr[2]) {
            return arr[1]; // arr[1] is unique
        } else {
            return arr[0]; // arr[0] is unique
        }
    } else {
        // The first two numbers are the same, so this is the common number.
        // We iterate through the rest of the array to find the different one.
        double commonNumber = arr[0];
        for (size_t i = 2; i < size; ++i) {
            if (arr[i] != commonNumber) {
                return arr[i];
            }
        }
    }
    // This part should not be reachable given the problem constraints
    return -1.0;
}

int main() {
    // Test cases
    double test1[] = {1, 1, 1, 2, 1, 1};
    printf("%g\n", findUniq(test1, sizeof(test1) / sizeof(test1[0])));

    double test2[] = {0, 0, 0.55, 0, 0};
    printf("%g\n", findUniq(test2, sizeof(test2) / sizeof(test2[0])));

    double test3[] = {3, 10, 3, 3, 3};
    printf("%g\n", findUniq(test3, sizeof(test3) / sizeof(test3[0])));

    double test4[] = {5, 5, 5, 5, 4};
    printf("%g\n", findUniq(test4, sizeof(test4) / sizeof(test4[0])));

    double test5[] = {8, 8, 8, 8, 8, 8, 8, 7};
    printf("%g\n", findUniq(test5, sizeof(test5) / sizeof(test5[0])));

    return 0;
}