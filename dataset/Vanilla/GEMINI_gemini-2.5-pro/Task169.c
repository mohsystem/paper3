#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int val;
    int index;
} Pair;

void merge(Pair* items, int start, int mid, int end, int* counts, Pair* temp_storage) {
    int i = start;
    int j = mid + 1;
    int k = 0;
    int rightCounter = 0;

    while (i <= mid && j <= end) {
        if (items[i].val <= items[j].val) {
            counts[items[i].index] += rightCounter;
            temp_storage[k++] = items[i++];
        } else {
            rightCounter++;
            temp_storage[k++] = items[j++];
        }
    }

    while (i <= mid) {
        counts[items[i].index] += rightCounter;
        temp_storage[k++] = items[i++];
    }
    while (j <= end) {
        temp_storage[k++] = items[j++];
    }

    for (int l = 0; l < k; l++) {
        items[start + l] = temp_storage[l];
    }
}

void mergeSort(Pair* items, int start, int end, int* counts, Pair* temp) {
    if (start >= end) {
        return;
    }
    int mid = start + (end - start) / 2;
    mergeSort(items, start, mid, counts, temp);
    mergeSort(items, mid + 1, end, counts, temp);
    merge(items, start, mid, end, counts, temp);
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }
    
    *returnSize = numsSize;
    Pair* items = (Pair*)malloc(numsSize * sizeof(Pair));
    for (int i = 0; i < numsSize; ++i) {
        items[i].val = nums[i];
        items[i].index = i;
    }

    int* counts = (int*)calloc(numsSize, sizeof(int));
    Pair* temp = (Pair*)malloc(numsSize * sizeof(Pair));

    mergeSort(items, 0, numsSize - 1, counts, temp);

    free(items);
    free(temp);
    return counts;
}

void printArray(int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]");
}

void runTestCase(int* nums, int size, const char* testName) {
    printf("%s:\nInput: ", testName);
    printArray(nums, size);
    
    int returnSize;
    int* result = countSmaller(nums, size, &returnSize);

    printf("\nOutput: ");
    printArray(result, returnSize);
    printf("\n\n");

    free(result);
}

int main() {
    // Test Case 1
    int nums1[] = {5, 2, 6, 1};
    runTestCase(nums1, sizeof(nums1)/sizeof(nums1[0]), "Test Case 1");

    // Test Case 2
    int nums2[] = {-1};
    runTestCase(nums2, sizeof(nums2)/sizeof(nums2[0]), "Test Case 2");

    // Test Case 3
    int nums3[] = {-1, -1};
    runTestCase(nums3, sizeof(nums3)/sizeof(nums3[0]), "Test Case 3");
    
    // Test Case 4
    int nums4[] = {2, 0, 1};
    runTestCase(nums4, sizeof(nums4)/sizeof(nums4[0]), "Test Case 4");

    // Test Case 5
    int* nums5 = NULL;
    runTestCase(nums5, 0, "Test Case 5");

    return 0;
}