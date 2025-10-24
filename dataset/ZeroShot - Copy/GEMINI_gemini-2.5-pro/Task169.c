#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int val;
    int index;
} Pair;

void merge(Pair* pairs, int start, int mid, int end, int* counts);
void merge_sort(Pair* pairs, int start, int end, int* counts);

int* countSmaller(int* nums, int numsSize, int* returnSize) {
    if (nums == NULL || numsSize == 0) {
        *returnSize = 0;
        return NULL;
    }

    int* counts = (int*)calloc(numsSize, sizeof(int));
    if (counts == NULL) {
        *returnSize = 0;
        return NULL; 
    }
    
    Pair* pairs = (Pair*)malloc(numsSize * sizeof(Pair));
    if (pairs == NULL) {
        free(counts);
        *returnSize = 0;
        return NULL;
    }
    
    for (int i = 0; i < numsSize; i++) {
        pairs[i].val = nums[i];
        pairs[i].index = i;
    }

    merge_sort(pairs, 0, numsSize - 1, counts);

    free(pairs);
    
    *returnSize = numsSize;
    return counts;
}

void merge_sort(Pair* pairs, int start, int end, int* counts) {
    if (start >= end) {
        return;
    }

    int mid = start + (end - start) / 2;
    merge_sort(pairs, start, mid, counts);
    merge_sort(pairs, mid + 1, end, counts);
    merge(pairs, start, mid, end, counts);
}

void merge(Pair* pairs, int start, int mid, int end, int* counts) {
    int tempSize = end - start + 1;
    Pair* temp = (Pair*)malloc(tempSize * sizeof(Pair));
    if (temp == NULL) {
        perror("Failed to allocate memory in merge");
        exit(EXIT_FAILURE); 
    }

    int i = start;
    int j = mid + 1;
    int k = 0;
    int rightElementsSmaller = 0;

    while (i <= mid && j <= end) {
        if (pairs[i].val > pairs[j].val) {
            temp[k++] = pairs[j++];
            rightElementsSmaller++;
        } else {
            counts[pairs[i].index] += rightElementsSmaller;
            temp[k++] = pairs[i++];
        }
    }

    while (i <= mid) {
        counts[pairs[i].index] += rightElementsSmaller;
        temp[k++] = pairs[i++];
    }

    while (j <= end) {
        temp[k++] = pairs[j++];
    }

    memcpy(pairs + start, temp, tempSize * sizeof(Pair));

    free(temp);
}

void print_array(const int* arr, int size) {
    printf("[");
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1 ? "" : ", "));
    }
    printf("]\n");
}

int main() {
    // Test Case 1
    int nums1[] = {5, 2, 6, 1};
    int size1 = sizeof(nums1) / sizeof(nums1[0]);
    int returnSize1;
    int* result1 = countSmaller(nums1, size1, &returnSize1);
    printf("Test Case 1: ");
    print_array(result1, returnSize1);
    free(result1);

    // Test Case 2
    int nums2[] = {-1};
    int size2 = sizeof(nums2) / sizeof(nums2[0]);
    int returnSize2;
    int* result2 = countSmaller(nums2, size2, &returnSize2);
    printf("Test Case 2: ");
    print_array(result2, returnSize2);
    free(result2);

    // Test Case 3
    int nums3[] = {-1, -1};
    int size3 = sizeof(nums3) / sizeof(nums3[0]);
    int returnSize3;
    int* result3 = countSmaller(nums3, size3, &returnSize3);
    printf("Test Case 3: ");
    print_array(result3, returnSize3);
    free(result3);
    
    // Test Case 4
    int* nums4 = NULL;
    int size4 = 0;
    int returnSize4;
    int* result4 = countSmaller(nums4, size4, &returnSize4);
    printf("Test Case 4: ");
    print_array(result4, returnSize4);
    free(result4);

    // Test Case 5
    int nums5[] = {2, 0, 1};
    int size5 = sizeof(nums5) / sizeof(nums5[0]);
    int returnSize5;
    int* result5 = countSmaller(nums5, size5, &returnSize5);
    printf("Test Case 5: ");
    print_array(result5, returnSize5);
    free(result5);

    return 0;
}