#include <stdio.h>
#include <stdlib.h>

// Comparison function for qsort
int compare_integers(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

/**
 * Sorts the positive numbers in an array in ascending order,
 * while keeping the negative numbers in their original positions.
 * NOTE: Returns a new dynamically allocated array that must be freed by the caller.
 *
 * @param arr The input array of integers.
 * @param size The number of elements in the array.
 * @return A new array with positive numbers sorted, or NULL on allocation failure.
 */
int* posNegSort(const int* arr, int size) {
    if (arr == NULL || size <= 0) {
        int* empty_result = (int*)malloc(0); // Return a valid, freeable pointer for size 0
        return empty_result;
    }

    // 1. Count and extract positive numbers
    int pos_count = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            pos_count++;
        }
    }

    int* positives = (int*)malloc(pos_count * sizeof(int));
    if (pos_count > 0 && positives == NULL) {
        return NULL; // Allocation failed
    }

    int current_pos = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            positives[current_pos++] = arr[i];
        }
    }

    // 2. Sort the positive numbers
    qsort(positives, pos_count, sizeof(int), compare_integers);

    // 3. Create the result array by placing sorted positives back
    int* result = (int*)malloc(size * sizeof(int));
    if (result == NULL) {
        free(positives);
        return NULL; // Allocation failed
    }
    
    current_pos = 0;
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            result[i] = positives[current_pos++];
        } else {
            result[i] = arr[i];
        }
    }

    free(positives);
    return result;
}

void printArray(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]");
}

int main() {
    // 5 test cases
    int test1[] = {6, 3, -2, 5, -8, 2, -2};
    int size1 = sizeof(test1) / sizeof(test1[0]);
    int* result1 = posNegSort(test1, size1);
    printf("Original: "); printArray(test1, size1); printf("\n");
    printf("Sorted:   "); printArray(result1, size1); printf("\n\n");
    free(result1);

    int test2[] = {6, 5, 4, -1, 3, 2, -1, 1};
    int size2 = sizeof(test2) / sizeof(test2[0]);
    int* result2 = posNegSort(test2, size2);
    printf("Original: "); printArray(test2, size2); printf("\n");
    printf("Sorted:   "); printArray(result2, size2); printf("\n\n");
    free(result2);

    int test3[] = {-5, -5, -5, -5, 7, -5};
    int size3 = sizeof(test3) / sizeof(test3[0]);
    int* result3 = posNegSort(test3, size3);
    printf("Original: "); printArray(test3, size3); printf("\n");
    printf("Sorted:   "); printArray(result3, size3); printf("\n\n");
    free(result3);

    int* test4 = NULL;
    int size4 = 0;
    int* result4 = posNegSort(test4, size4);
    printf("Original: "); printArray(test4, size4); printf("\n");
    printf("Sorted:   "); printArray(result4, size4); printf("\n\n");
    free(result4);

    int test5[] = {5, 4, 3, 2, 1};
    int size5 = sizeof(test5) / sizeof(test5[0]);
    int* result5 = posNegSort(test5, size5);
    printf("Original: "); printArray(test5, size5); printf("\n");
    printf("Sorted:   "); printArray(result5, size5); printf("\n\n");
    free(result5);

    return 0;
}