#include <stdio.h>
#include <stdlib.h>

// This is the MountainArray's API interface.
// You should not implement it, or speculate about its implementation
typedef struct MountainArray MountainArray;
struct MountainArray {
    int* arr;
    int size;
    int (*get)(MountainArray*, int);
    int (*length)(MountainArray*);
};

// Functions to interact with the MountainArray
int mountain_array_get(MountainArray* mountainArr, int index) {
    return mountainArr->arr[index];
}

int mountain_array_length(MountainArray* mountainArr) {
    return mountainArr->size;
}

// Function to create a MountainArray for testing
MountainArray* createMountainArray(int* arr_data, int size) {
    MountainArray* ma = (MountainArray*)malloc(sizeof(MountainArray));
    if (ma == NULL) return NULL;
    ma->arr = arr_data;
    ma->size = size;
    ma->get = mountain_array_get;
    ma->length = mountain_array_length;
    return ma;
}

/**
 * *********************************************************************
 * // This is the MountainArray's API interface.
 * // You should not implement it, or speculate about its implementation
 * *********************************************************************
 *
 * int get(MountainArray *, int index);
 * int length(MountainArray *);
 */

int findInMountainArray(int target, MountainArray* mountainArr) {
    int length = mountainArr->length(mountainArr);

    // 1. Find the peak index
    int low = 0, high = length - 1;
    int peakIndex = -1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (mountainArr->get(mountainArr, mid) < mountainArr->get(mountainArr, mid + 1)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    peakIndex = low;

    // 2. Search in the ascending part [0, peakIndex]
    low = 0;
    high = peakIndex;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr->get(mountainArr, mid);
        if (val == target) {
            return mid;
        } else if (val < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    // 3. Search in the descending part [peakIndex + 1, length - 1]
    low = peakIndex + 1;
    high = length - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr->get(mountainArr, mid);
        if (val == target) {
            return mid;
        } else if (val < target) { // descending
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    return -1;
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArray* mountainArr1 = createMountainArray(arr1, sizeof(arr1)/sizeof(arr1[0]));
    int target1 = 3;
    printf("Test Case 1: %d\n", findInMountainArray(target1, mountainArr1));
    free(mountainArr1);

    // Test Case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    MountainArray* mountainArr2 = createMountainArray(arr2, sizeof(arr2)/sizeof(arr2[0]));
    int target2 = 3;
    printf("Test Case 2: %d\n", findInMountainArray(target2, mountainArr2));
    free(mountainArr2);

    // Test Case 3
    int arr3[] = {1, 5, 2};
    MountainArray* mountainArr3 = createMountainArray(arr3, sizeof(arr3)/sizeof(arr3[0]));
    int target3 = 2;
    printf("Test Case 3: %d\n", findInMountainArray(target3, mountainArr3));
    free(mountainArr3);
    
    // Test Case 4
    int arr4[] = {1, 2, 5, 2, 1};
    MountainArray* mountainArr4 = createMountainArray(arr4, sizeof(arr4)/sizeof(arr4[0]));
    int target4 = 1;
    printf("Test Case 4: %d\n", findInMountainArray(target4, mountainArr4));
    free(mountainArr4);

    // Test Case 5
    int arr5[] = {1, 2, 5, 2, 1};
    MountainArray* mountainArr5 = createMountainArray(arr5, sizeof(arr5)/sizeof(arr5[0]));
    int target5 = 5;
    printf("Test Case 5: %d\n", findInMountainArray(target5, mountainArr5));
    free(mountainArr5);

    return 0;
}