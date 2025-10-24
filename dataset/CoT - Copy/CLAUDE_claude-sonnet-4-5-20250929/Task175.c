
#include <stdio.h>
#include <stdlib.h>

// MountainArray interface (simulated with function pointers)
typedef struct {
    int* arr;
    int size;
} MountainArrayData;

int mountain_array_get(MountainArrayData* data, int index) {
    if (index >= 0 && index < data->size) {
        return data->arr[index];
    }
    return -1;
}

int mountain_array_length(MountainArrayData* data) {
    return data->size;
}

int find_peak(MountainArrayData* data, int length) {
    int left = 0;
    int right = length - 1;
    
    while (left < right) {
        int mid = left + (right - left) / 2;
        int mid_val = mountain_array_get(data, mid);
        int next_val = mountain_array_get(data, mid + 1);
        
        if (mid_val < next_val) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}

int binary_search_ascending(MountainArrayData* data, int target, int left, int right) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int mid_val = mountain_array_get(data, mid);
        
        if (mid_val == target) {
            return mid;
        } else if (mid_val < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

int binary_search_descending(MountainArrayData* data, int target, int left, int right) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int mid_val = mountain_array_get(data, mid);
        
        if (mid_val == target) {
            return mid;
        } else if (mid_val > target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

int findInMountainArray(int target, MountainArrayData* data) {
    // Input validation
    if (data == NULL || data->arr == NULL) {
        return -1;
    }
    
    int length = mountain_array_length(data);
    if (length < 3) {
        return -1;
    }
    
    // Step 1: Find the peak index
    int peak_index = find_peak(data, length);
    
    // Step 2: Search in the ascending part (left side)
    int result = binary_search_ascending(data, target, 0, peak_index);
    
    // Step 3: If not found, search in the descending part (right side)
    if (result == -1) {
        result = binary_search_descending(data, target, peak_index + 1, length - 1);
    }
    
    return result;
}

int main() {
    // Test case 1
    int arr1[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArrayData test1 = {arr1, 7};
    printf("Test 1: %d\\n", findInMountainArray(3, &test1)); // Expected: 2
    
    // Test case 2
    int arr2[] = {0, 1, 2, 4, 2, 1};
    MountainArrayData test2 = {arr2, 6};
    printf("Test 2: %d\\n", findInMountainArray(3, &test2)); // Expected: -1
    
    // Test case 3
    int arr3[] = {1, 2, 3, 4, 5, 3, 1};
    MountainArrayData test3 = {arr3, 7};
    printf("Test 3: %d\\n", findInMountainArray(5, &test3)); // Expected: 4
    
    // Test case 4
    int arr4[] = {0, 5, 3, 1};
    MountainArrayData test4 = {arr4, 4};
    printf("Test 4: %d\\n", findInMountainArray(1, &test4)); // Expected: 3
    
    // Test case 5
    int arr5[] = {1, 5, 2};
    MountainArrayData test5 = {arr5, 3};
    printf("Test 5: %d\\n", findInMountainArray(2, &test5)); // Expected: 2
    
    return 0;
}
