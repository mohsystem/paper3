#include <stdio.h>
#include <stdlib.h>

// Comparison function required by qsort
int compare_integers(const void* a, const void* b) {
    int int_a = *((const int*)a);
    int int_b = *((const int*)b);
    if (int_a == int_b) return 0;
    return (int_a < int_b) ? -1 : 1;
}

/**
 * Sorts the positive numbers in an array in-place,
 * keeping negative numbers untouched.
 *
 * @param arr Pointer to the integer array.
 * @param size The number of elements in the array.
 */
void posNegSort(int* arr, int size) {
    // Handle null pointer or empty array.
    if (arr == NULL || size <= 0) {
        return;
    }

    // 1. Count the number of positive elements.
    int pos_count = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            pos_count++;
        }
    }
    
    // If there are no positive numbers, there is nothing to sort.
    if (pos_count == 0) {
        return;
    }

    // 2. Allocate memory for positives and extract them.
    int* positives = (int*)malloc(pos_count * sizeof(int));
    if (positives == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return; 
    }
    
    int current_pos_index = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            positives[current_pos_index++] = arr[i];
        }
    }

    // 3. Sort the array of positive numbers.
    qsort(positives, pos_count, sizeof(int), compare_integers);

    // 4. Place the sorted positive numbers back into the original array.
    current_pos_index = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            arr[i] = positives[current_pos_index++];
        }
    }

    // 5. Free the dynamically allocated memory.
    free(positives);
}

// Helper function to print an array
void print_array(const char* label, const int* arr, int size) {
    printf("%s", label);
    if (size == 0) {
        printf("[]");
    }
    for (int i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1
    int test1[] = {6, 3, -2, 5, -8, 2, -2};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    printf("Test Case 1:\n");
    print_array("Original: ", test1, size1);
    posNegSort(test1, size1);
    print_array("Sorted:   ", test1, size1);
    printf("\n");

    // Test Case 2
    int test2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    printf("Test Case 2:\n");
    print_array("Original: ", test2, size2);
    posNegSort(test2, size2);
    print_array("Sorted:   ", test2, size2);
    printf("\n");

    // Test Case 3
    int test3[] = {-5, -5, -5, -5, 7, -5};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    printf("Test Case 3:\n");
    print_array("Original: ", test3, size3);
    posNegSort(test3, size3);
    print_array("Sorted:   ", test3, size3);
    printf("\n");

    // Test Case 4 (Empty array)
    int test4[] = {};
    int size4 = sizeof(test4) / sizeof(test4[0]);
    printf("Test Case 4:\n");
    print_array("Original: ", test4, size4);
    posNegSort(test4, size4);
    print_array("Sorted:   ", test4, size4);
    printf("\n");
    
    // Test Case 5
    int test5[] = {5, -1, 4, -2, 3, -3, 2, -4, 1};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    printf("Test Case 5:\n");
    print_array("Original: ", test5, size5);
    posNegSort(test5, size5);
    print_array("Sorted:   ", test5, size5);
    printf("\n");

    return 0;
}