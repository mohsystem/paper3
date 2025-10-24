#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int value;
    int index;
} Item;

void merge(Item* items, int* counts, int start, int mid, int end);
void mergeSort(Item* items, int* counts, int start, int end);

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }

    *returnSize = numsSize;
    int* counts = (int*)calloc(numsSize, sizeof(int));
    if (counts == NULL) {
        *returnSize = 0;
        return NULL;
    }

    Item* items = (Item*)malloc(numsSize * sizeof(Item));
    if (items == NULL) {
        free(counts);
        *returnSize = 0;
        return NULL;
    }

    for (int i = 0; i < numsSize; i++) {
        items[i].value = nums[i];
        items[i].index = i;
    }

    mergeSort(items, counts, 0, numsSize - 1);
    
    free(items);
    return counts;
}

void mergeSort(Item* items, int* counts, int start, int end) {
    if (start >= end) {
        return;
    }
    int mid = start + (end - start) / 2;
    mergeSort(items, counts, start, mid);
    mergeSort(items, counts, mid + 1, end);
    merge(items, counts, start, mid, end);
}

void merge(Item* items, int* counts, int start, int mid, int end) {
    int len = end - start + 1;
    Item* temp = (Item*)malloc(len * sizeof(Item));
    if (temp == NULL) {
        // This is a fatal error for this algorithm's context.
        // A robust application would handle this more gracefully.
        exit(EXIT_FAILURE);
    }

    int i = start;
    int j = mid + 1;
    int k = 0;

    while (i <= mid && j <= end) {
        if (items[i].value <= items[j].value) {
            counts[items[i].index] += j - (mid + 1);
            temp[k++] = items[i++];
        } else {
            temp[k++] = items[j++];
        }
    }

    while (i <= mid) {
        counts[items[i].index] += j - (mid + 1);
        temp[k++] = items[i++];
    }

    while (j <= end) {
        temp[k++] = items[j++];
    }

    memcpy(items + start, temp, len * sizeof(Item));
    free(temp);
}

void print_array(const char* prefix, int* arr, int size) {
    printf("%s", prefix);
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

void run_test_case(int test_num, int* nums, int size) {
    printf("Test Case %d:\n", test_num);
    print_array("Input: ", nums, size);
    int returnSize;
    int* result = countSmaller(nums, size, &returnSize);
    print_array("Output: ", result, returnSize);
    free(result);
    printf("\n");
}

int main() {
    // Test Case 1
    int nums1[] = {5, 2, 6, 1};
    run_test_case(1, nums1, sizeof(nums1)/sizeof(nums1[0]));

    // Test Case 2
    int nums2[] = {-1};
    run_test_case(2, nums2, sizeof(nums2)/sizeof(nums2[0]));

    // Test Case 3
    int nums3[] = {-1, -1};
    run_test_case(3, nums3, sizeof(nums3)/sizeof(nums3[0]));

    // Test Case 4
    int* nums4 = NULL;
    run_test_case(4, nums4, 0);

    // Test Case 5
    int nums5[] = {2, 0, 1};
    run_test_case(5, nums5, sizeof(nums5)/sizeof(nums5[0]));

    return 0;
}