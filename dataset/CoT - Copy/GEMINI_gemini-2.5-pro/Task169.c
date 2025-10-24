#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int value;
    int index;
} Item;

void merge(Item* items, int left, int mid, int right, int* counts) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Item* leftArr = (Item*)malloc(n1 * sizeof(Item));
    Item* rightArr = (Item*)malloc(n2 * sizeof(Item));
    
    if (leftArr == NULL || rightArr == NULL) {
        if(leftArr) free(leftArr);
        if(rightArr) free(rightArr);
        return; 
    }
    
    for(int i = 0; i < n1; i++) leftArr[i] = items[left + i];
    for(int j = 0; j < n2; j++) rightArr[j] = items[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (leftArr[i].value <= rightArr[j].value) {
            counts[leftArr[i].index] += j;
            items[k++] = leftArr[i++];
        } else {
            items[k++] = rightArr[j++];
        }
    }
    while (i < n1) {
        counts[leftArr[i].index] += j;
        items[k++] = leftArr[i++];
    }
    while (j < n2) {
        items[k++] = rightArr[j++];
    }
    
    free(leftArr);
    free(rightArr);
}

void mergeSort(Item* items, int left, int right, int* counts) {
    if (left >= right) {
        return;
    }
    int mid = left + (right - left) / 2;
    mergeSort(items, left, mid, counts);
    mergeSort(items, mid + 1, right, counts);
    merge(items, left, mid, right, counts);
}

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }

    Item* items = (Item*)malloc(numsSize * sizeof(Item));
    int* counts = (int*)calloc(numsSize, sizeof(int));
    if (items == NULL || counts == NULL) {
        *returnSize = 0;
        free(items);
        free(counts);
        return NULL;
    }

    for (int i = 0; i < numsSize; i++) {
        items[i].value = nums[i];
        items[i].index = i;
    }

    mergeSort(items, 0, numsSize - 1, counts);

    free(items);
    *returnSize = numsSize;
    return counts;
}

void print_array(int* arr, int size) {
    if (size == 0) {
        printf("[]\n");
        return;
    }
    printf("[");
    for (int i = 0; i < size; i++) {
        printf("%d", arr[i]);
        if (i < size - 1) {
            printf(", ");
        }
    }
    printf("]\n");
}

int main() {
    int returnSize;

    // Test Case 1
    int nums1[] = {5, 2, 6, 1};
    int* result1 = countSmaller(nums1, 4, &returnSize);
    printf("Test Case 1 Output: ");
    print_array(result1, returnSize);
    free(result1);

    // Test Case 2
    int nums2[] = {-1};
    int* result2 = countSmaller(nums2, 1, &returnSize);
    printf("Test Case 2 Output: ");
    print_array(result2, returnSize);
    free(result2);

    // Test Case 3
    int nums3[] = {-1, -1};
    int* result3 = countSmaller(nums3, 2, &returnSize);
    printf("Test Case 3 Output: ");
    print_array(result3, returnSize);
    free(result3);
    
    // Test Case 4
    int nums4[] = {5, 4, 3, 2, 1};
    int* result4 = countSmaller(nums4, 5, &returnSize);
    printf("Test Case 4 Output: ");
    print_array(result4, returnSize);
    free(result4);

    // Test Case 5
    int nums5[] = {};
    int* result5 = countSmaller(nums5, 0, &returnSize);
    printf("Test Case 5 Output: ");
    print_array(result5, returnSize);
    free(result5);

    return 0;
}