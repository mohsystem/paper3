#include <stdio.h>

int find_even_index(const int* arr, int size) {
    long long total_sum = 0;
    for (int i = 0; i < size; i++) {
        total_sum += arr[i];
    }

    long long left_sum = 0;
    for (int i = 0; i < size; i++) {
        // The right sum is the total sum minus the left sum and the current element
        long long right_sum = total_sum - left_sum - arr[i];
        if (left_sum == right_sum) {
            return i;
        }
        left_sum += arr[i];
    }

    return -1;
}

void print_array(const int* arr, int size) {
    printf("{");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("}");
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 4, 3, 2, 1};
    int size1 = sizeof(arr1) / sizeof(arr1[0]);
    printf("Array: ");
    print_array(arr1, size1);
    printf(" -> Index: %d\n", find_even_index(arr1, size1));

    // Test Case 2
    int arr2[] = {1, 100, 50, -51, 1, 1};
    int size2 = sizeof(arr2) / sizeof(arr2[0]);
    printf("Array: ");
    print_array(arr2, size2);
    printf(" -> Index: %d\n", find_even_index(arr2, size2));
    
    // Test Case 3
    int arr3[] = {20, 10, -80, 10, 10, 15, 35};
    int size3 = sizeof(arr3) / sizeof(arr3[0]);
    printf("Array: ");
    print_array(arr3, size3);
    printf(" -> Index: %d\n", find_even_index(arr3, size3));

    // Test Case 4
    int arr4[] = {10, -80, 10, 10, 15, 35, 20};
    int size4 = sizeof(arr4) / sizeof(arr4[0]);
    printf("Array: ");
    print_array(arr4, size4);
    printf(" -> Index: %d\n", find_even_index(arr4, size4));

    // Test Case 5
    int arr5[] = {1, 2, 3, 4, 5, 6};
    int size5 = sizeof(arr5) / sizeof(arr5[0]);
    printf("Array: ");
    print_array(arr5, size5);
    printf(" -> Index: %d\n", find_even_index(arr5, size5));

    return 0;
}