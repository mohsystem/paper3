
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int* arr;
    int size;
} MountainArray;

int get(MountainArray* ma, int index) {
    return ma->arr[index];
}

int length(MountainArray* ma) {
    return ma->size;
}

int findPeak(MountainArray* mountainArr) {
    int left = 0, right = length(mountainArr) - 1;
    while (left < right) {
        int mid = left + (right - left) / 2;
        if (get(mountainArr, mid) < get(mountainArr, mid + 1)) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

int binarySearch(MountainArray* mountainArr, int target, int left, int right, int ascending) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int midVal = get(mountainArr, mid);
        
        if (midVal == target) {
            return mid;
        }
        
        if (ascending) {
            if (midVal < target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        } else {
            if (midVal > target) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
    }
    return -1;
}

int findInMountainArray(int target, MountainArray* mountainArr) {
    int len = length(mountainArr);
    
    int peakIndex = findPeak(mountainArr);
    
    int index = binarySearch(mountainArr, target, 0, peakIndex, 1);
    if (index != -1) {
        return index;
    }
    
    return binarySearch(mountainArr, target, peakIndex + 1, len - 1, 0);
}

int main() {
    // Test case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma1 = {arr1, 7};
    printf("Test 1: %d\\n", findInMountainArray(3, &ma1));
    
    // Test case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    MountainArray ma2 = {arr2, 6};
    printf("Test 2: %d\\n", findInMountainArray(3, &ma2));
    
    // Test case 3
    int arr3[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma3 = {arr3, 7};
    printf("Test 3: %d\\n", findInMountainArray(5, &ma3));
    
    // Test case 4
    int arr4[] = {0, 5, 3, 1};
    MountainArray ma4 = {arr4, 4};
    printf("Test 4: %d\\n", findInMountainArray(1, &ma4));
    
    // Test case 5
    int arr5[] = {1, 5, 2};
    MountainArray ma5 = {arr5, 3};
    printf("Test 5: %d\\n", findInMountainArray(2, &ma5));
    
    return 0;
}
