#include <stdio.h>

/**
 * @brief Finds the integer that appears an odd number of times in an array.
 *
 * It uses the XOR bitwise operator. The property of XOR is that a^a = 0 and a^0 = a.
 * When we XOR all numbers, pairs of identical numbers cancel out, leaving the unique one.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return The integer that appears an odd number of times.
 */
int findOdd(const int arr[], int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        result ^= arr[i];
    }
    return result;
}

void printArray(const int arr[], int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]");
}

int main() {
    // Test Case 1
    int test1[] = {7};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Input: "); printArray(test1, size1); printf(", Output: %d\n", findOdd(test1, size1));

    // Test Case 2
    int test2[] = {0};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Input: "); printArray(test2, size2); printf(", Output: %d\n", findOdd(test2, size2));

    // Test Case 3
    int test3[] = {1, 1, 2};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Input: "); printArray(test3, size3); printf(", Output: %d\n", findOdd(test3, size3));

    // Test Case 4
    int test4[] = {0, 1, 0, 1, 0};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Input: "); printArray(test4, size4); printf(", Output: %d\n", findOdd(test4, size4));

    // Test Case 5
    int test5[] = {1, 2, 2, 3, 3, 3, 4, 3, 3, 3, 2, 2, 1};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Input: "); printArray(test5, size5); printf(", Output: %d\n", findOdd(test5, size5));

    return 0;
}