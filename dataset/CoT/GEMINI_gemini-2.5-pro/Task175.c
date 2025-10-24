#include <stdio.h>
#include <stdlib.h>

// Mock structure and functions for local testing
typedef struct {
    int *arr;
    int size;
} MountainArray;

int get(MountainArray* mountainArr, int index) {
    return mountainArr->arr[index];
}

int length(MountainArray* mountainArr) {
    return mountainArr->size;
}

// The main solution function
int findInMountainArray(int target, MountainArray* mountainArr) {
    int len = length(mountainArr);

    // 1. Find the index of the peak element
    int low = 0, high = len - 1;
    int peakIndex = -1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (get(mountainArr, mid) < get(mountainArr, mid + 1)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    peakIndex = low;

    // 2. Search in the strictly increasing part
    low = 0;
    high = peakIndex;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int midVal = get(mountainArr, mid);
        if (midVal == target) {
            return mid;
        } else if (midVal < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    // 3. If not found, search in the strictly decreasing part
    low = peakIndex + 1;
    high = len - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int midVal = get(mountainArr, mid);
        if (midVal == target) {
            return mid;
        } else if (midVal > target) { // Note the change in logic
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    // 4. Target not found
    return -1;
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArray mountainArr1 = {arr1, sizeof(arr1)/sizeof(arr1[0])};
    int target1 = 3;
    printf("Test Case 1: %d\n", findInMountainArray(target1, &mountainArr1)); // Expected: 2

    // Test Case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    MountainArray mountainArr2 = {arr2, sizeof(arr2)/sizeof(arr2[0])};
    int target2 = 3;
    printf("Test Case 2: %d\n", findInMountainArray(target2, &mountainArr2)); // Expected: -1

    // Test Case 3
    int arr3[] = {1, 5, 2};
    MountainArray mountainArr3 = {arr3, sizeof(arr3)/sizeof(arr3[0])};
    int target3 = 2;
    printf("Test Case 3: %d\n", findInMountainArray(target3, &mountainArr3)); // Expected: 2

    // Test Case 4
    int arr4[] = {1, 2, 3, 4, 5, 6, 7};
    MountainArray mountainArr4 = {arr4, sizeof(arr4)/sizeof(arr4[0])};
    int target4 = 5;
    printf("Test Case 4: %d\n", findInMountainArray(target4, &mountainArr4)); // Expected: 4

    // Test Case 5
    int arr5[] = {7, 6, 5, 4, 3, 2, 1};
    MountainArray mountainArr5 = {arr5, sizeof(arr5)/sizeof(arr5[0])};
    int target5 = 5;
    printf("Test Case 5: %d\n", findInMountainArray(target5, &mountainArr5)); // Expected: 2

    return 0;
}