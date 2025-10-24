#include <stdio.h>
#include <stdlib.h>

// Forward declaration of the MountainArray struct
struct MountainArray;

// Typedefs for function pointers to simulate methods
typedef int (*GET_FUNC)(struct MountainArray* obj, int index);
typedef int (*LENGTH_FUNC)(struct MountainArray* obj);

// The MountainArray structure to hold data and function pointers
typedef struct MountainArray {
    int* data;
    int size;
    GET_FUNC get;
    LENGTH_FUNC length;
} MountainArray;

// Implementation of the 'get' method
int mountain_array_get_impl(MountainArray* obj, int index) {
    if (obj && index >= 0 && index < obj->size) {
        return obj->data[index];
    }
    return -1; // Should not happen in valid calls
}

// Implementation of the 'length' method
int mountain_array_length_impl(MountainArray* obj) {
    return obj ? obj->size : 0;
}

// Function to create a mock MountainArray for testing
MountainArray* createMountainArray(int* data, int size) {
    MountainArray* ma = (MountainArray*)malloc(sizeof(MountainArray));
    if (!ma) return NULL;
    ma->data = data;
    ma->size = size;
    ma->get = mountain_array_get_impl;
    ma->length = mountain_array_length_impl;
    return ma;
}

// The core solution function
int findInMountainArray(int target, MountainArray* mountainArr) {
    int len = mountainArr->length(mountainArr);

    // 1. Find the index of the peak element
    int low = 0, high = len - 1;
    while (low < high) {
        int mid = low + (high - low) / 2;
        if (mountainArr->get(mountainArr, mid) < mountainArr->get(mountainArr, mid + 1)) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    int peak = low;

    // 2. Search in the left (ascending) subarray
    low = 0;
    high = peak;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr->get(mountainArr, mid);
        if (val < target) {
            low = mid + 1;
        } else if (val > target) {
            high = mid - 1;
        } else {
            return mid;
        }
    }

    // 3. Search in the right (descending) subarray
    low = peak + 1;
    high = len - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int val = mountainArr->get(mountainArr, mid);
        if (val > target) { // Note the change for descending order
            low = mid + 1;
        } else if (val < target) {
            high = mid - 1;
        } else {
            return mid;
        }
    }

    // 4. Target not found
    return -1;
}

void run_test_case(const char* name, int arr[], int size, int target) {
    MountainArray* ma = createMountainArray(arr, size);
    if (ma) {
        int result = findInMountainArray(target, ma);
        printf("%s: %d\n", name, result);
        free(ma);
    }
}

int main() {
    // Test Case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    run_test_case("Test Case 1", arr1, sizeof(arr1)/sizeof(arr1[0]), 3);

    // Test Case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    run_test_case("Test Case 2", arr2, sizeof(arr2)/sizeof(arr2[0]), 3);

    // Test Case 3
    int arr3[] = {1, 5, 2};
    run_test_case("Test Case 3", arr3, sizeof(arr3)/sizeof(arr3[0]), 2);

    // Test Case 4
    int arr4[] = {1, 2, 5, 4, 3};
    run_test_case("Test Case 4", arr4, sizeof(arr4)/sizeof(arr4[0]), 5);
    
    // Test Case 5
    int arr5[] = {3, 5, 3, 2, 0};
    run_test_case("Test Case 5", arr5, sizeof(arr5)/sizeof(arr5[0]), 0);

    return 0;
}