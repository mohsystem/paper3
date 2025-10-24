#include <stdio.h>
#include <stdlib.h>

// Forward declaration of the MountainArray struct for use in function pointer types.
struct MountainArray;

// Define function pointer types for the "methods".
typedef int (*get_func)(struct MountainArray* self, int index);
typedef int (*length_func)(struct MountainArray* self);

// The MountainArray "interface" struct.
typedef struct MountainArray {
    void* p_impl; // Pointer to the actual implementation data.
    get_func get;
    length_func length;
} MountainArray;

// "Implementation" struct for local testing.
typedef struct {
    int* arr;
    int size;
} MountainArrayImpl;

// "Method" implementations that operate on MountainArrayImpl.
int mountain_array_impl_get(MountainArray* self, int index) {
    MountainArrayImpl* impl = (MountainArrayImpl*)self->p_impl;
    return impl->arr[index];
}

int mountain_array_impl_length(MountainArray* self) {
    MountainArrayImpl* impl = (MountainArrayImpl*)self->p_impl;
    return impl->size;
}

// "Constructor" to initialize a MountainArray object for testing.
void MountainArray_create(MountainArray* ma, MountainArrayImpl* impl, int* arr, int size) {
    impl->arr = arr;
    impl->size = size;
    ma->p_impl = impl;
    ma->get = mountain_array_impl_get;
    ma->length = mountain_array_impl_length;
}

int findInMountainArray(int target, MountainArray* mountainArr) {
    int length = mountainArr->length(mountainArr);

    // 1. Find the index of the peak element.
    int low = 0;
    int high = length - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (mountainArr->get(mountainArr, mid) < mountainArr->get(mountainArr, mid + 1)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    int peakIndex = low;

    // 2. Search in the left (strictly increasing) part of the array.
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

    // 3. If not found, search in the right (strictly decreasing) part.
    low = peakIndex + 1;
    high = length - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr->get(mountainArr, mid);
        if (val == target) {
            return mid;
        } else if (val < target) { // In a decreasing array, smaller values are to the right.
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    // 4. Target not found in the array.
    return -1;
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArray ma1;
    MountainArrayImpl impl1;
    MountainArray_create(&ma1, &impl1, arr1, sizeof(arr1)/sizeof(arr1[0]));
    int target1 = 3;
    printf("Test 1 Output: %d\n", findInMountainArray(target1, &ma1)); // Expected: 2

    // Test Case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    MountainArray ma2;
    MountainArrayImpl impl2;
    MountainArray_create(&ma2, &impl2, arr2, sizeof(arr2)/sizeof(arr2[0]));
    int target2 = 3;
    printf("Test 2 Output: %d\n", findInMountainArray(target2, &ma2)); // Expected: -1

    // Test Case 3
    int arr3[] = {1, 5, 2};
    MountainArray ma3;
    MountainArrayImpl impl3;
    MountainArray_create(&ma3, &impl3, arr3, sizeof(arr3)/sizeof(arr3[0]));
    int target3 = 2;
    printf("Test 3 Output: %d\n", findInMountainArray(target3, &ma3)); // Expected: 2
    
    // Test Case 4
    int arr4[] = {3, 5, 3, 2, 0};
    MountainArray ma4;
    MountainArrayImpl impl4;
    MountainArray_create(&ma4, &impl4, arr4, sizeof(arr4)/sizeof(arr4[0]));
    int target4 = 0;
    printf("Test 4 Output: %d\n", findInMountainArray(target4, &ma4)); // Expected: 4

    // Test Case 5
    int arr5[] = {1, 5, 2};
    MountainArray ma5;
    MountainArrayImpl impl5;
    MountainArray_create(&ma5, &impl5, arr5, sizeof(arr5)/sizeof(arr5[0]));
    int target5 = 5;
    printf("Test 5 Output: %d\n", findInMountainArray(target5, &ma5)); // Expected: 1

    return 0;
}