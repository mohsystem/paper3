
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_CALLS 100
#define MAX_ARRAY_SIZE 10000

// MountainArray structure for interactive problem
typedef struct {
    int* arr;
    int size;
    int callCount;
} MountainArray;

// Initialize MountainArray - validates size and allocates memory safely
// Security: validates size, checks malloc return, initializes all fields
MountainArray* createMountainArray(const int* array, int size) {
    // Validate size constraints
    if (size < 3 || size > MAX_ARRAY_SIZE || array == NULL) {
        return NULL;
    }
    
    // Allocate structure safely
    MountainArray* mountainArr = (MountainArray*)malloc(sizeof(MountainArray));
    if (mountainArr == NULL) {
        return NULL;
    }
    
    // Allocate array with size validation to prevent overflow
    if (size > INT_MAX / sizeof(int)) {
        free(mountainArr);
        return NULL;
    }
    
    mountainArr->arr = (int*)malloc(size * sizeof(int));
    if (mountainArr->arr == NULL) {
        free(mountainArr);
        return NULL;
    }
    
    // Copy array safely with bounds check
    memcpy(mountainArr->arr, array, size * sizeof(int));
    mountainArr->size = size;
    mountainArr->callCount = 0;
    
    return mountainArr;
}

// Get element at index with bounds checking and call limit
// Security: validates index bounds, tracks API calls
int mountainArrayGet(MountainArray* mountainArr, int index) {
    // Validate pointer
    if (mountainArr == NULL || mountainArr->arr == NULL) {
        return -1;
    }
    
    // Validate index bounds to prevent buffer overflow
    if (index < 0 || index >= mountainArr->size) {
        return -1;
    }
    
    // Track API calls to ensure limit not exceeded
    if (mountainArr->callCount >= MAX_CALLS) {
        return -1;
    }
    mountainArr->callCount++;
    
    return mountainArr->arr[index];
}

// Get array length safely
int mountainArrayLength(const MountainArray* mountainArr) {
    if (mountainArr == NULL) {
        return 0;
    }
    return mountainArr->size;
}

// Free MountainArray memory safely
// Security: checks for NULL, frees in correct order, prevents double-free
void freeMountainArray(MountainArray* mountainArr) {
    if (mountainArr != NULL) {
        if (mountainArr->arr != NULL) {
            free(mountainArr->arr);
            mountainArr->arr = NULL;
        }
        free(mountainArr);
    }
}

// Find peak index in mountain array with overflow protection
// Security: validates bounds, prevents integer overflow in calculations
int findPeak(MountainArray* mountainArr) {
    if (mountainArr == NULL) {
        return -1;
    }
    
    int left = 0;
    int length = mountainArrayLength(mountainArr);
    int right = length - 1;
    
    // Validate length
    if (length <= 0 || length > MAX_ARRAY_SIZE) {
        return -1;
    }
    
    while (left < right) {
        // Prevent integer overflow in mid calculation
        int mid = left + (right - left) / 2;
        
        // Validate mid is in valid range
        if (mid < 0 || mid >= length - 1) {
            break;
        }
        
        int midVal = mountainArrayGet(mountainArr, mid);
        int nextVal = mountainArrayGet(mountainArr, mid + 1);
        
        // Check for invalid return values
        if (midVal == -1 || nextVal == -1) {
            return -1;
        }
        
        if (midVal < nextVal) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    
    return left;
}

// Binary search in ascending part with bounds validation
// Security: all indices validated, overflow prevented
int binarySearchAsc(MountainArray* mountainArr, int target, int left, int right) {
    // Validate inputs
    if (mountainArr == NULL || left < 0 || right >= mountainArrayLength(mountainArr) || left > right) {
        return -1;
    }
    
    while (left <= right) {
        // Prevent overflow in mid calculation
        int mid = left + (right - left) / 2;
        
        // Validate mid before access
        if (mid < 0 || mid >= mountainArrayLength(mountainArr)) {
            return -1;
        }
        
        int midVal = mountainArrayGet(mountainArr, mid);
        
        // Check for invalid return
        if (midVal == -1 && mountainArr->callCount >= MAX_CALLS) {
            return -1;
        }
        
        if (midVal == target) {
            return mid;
        } else if (midVal < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

// Binary search in descending part with bounds validation
// Security: all indices validated, overflow prevented
int binarySearchDesc(MountainArray* mountainArr, int target, int left, int right) {
    // Validate inputs
    if (mountainArr == NULL || left < 0 || right >= mountainArrayLength(mountainArr) || left > right) {
        return -1;
    }
    
    while (left <= right) {
        // Prevent overflow in mid calculation
        int mid = left + (right - left) / 2;
        
        // Validate mid before access
        if (mid < 0 || mid >= mountainArrayLength(mountainArr)) {
            return -1;
        }
        
        int midVal = mountainArrayGet(mountainArr, mid);
        
        // Check for invalid return
        if (midVal == -1 && mountainArr->callCount >= MAX_CALLS) {
            return -1;
        }
        
        if (midVal == target) {
            return mid;
        } else if (midVal > target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return -1;
}

// Main solution function with full input validation
// Security: validates all inputs, handles edge cases safely
int findInMountainArray(int target, MountainArray* mountainArr) {
    // Validate input pointer
    if (mountainArr == NULL) {
        return -1;
    }
    
    int length = mountainArrayLength(mountainArr);
    
    // Validate length constraints
    if (length < 3 || length > MAX_ARRAY_SIZE) {
        return -1;
    }
    
    // Validate target range (problem constraint)
    if (target < 0 || target > 1000000000) {
        return -1;
    }
    
    // Find peak of mountain
    int peak = findPeak(mountainArr);
    
    // Validate peak index
    if (peak < 0 || peak >= length) {
        return -1;
    }
    
    // Search in ascending part first (to get minimum index)
    int result = binarySearchAsc(mountainArr, target, 0, peak);
    
    if (result != -1) {
        return result;
    }
    
    // Search in descending part if not found
    return binarySearchDesc(mountainArr, target, peak + 1, length - 1);
}

int main(void) {
    // Test case 1: target exists at multiple positions
    {
        int arr1[] = {1, 2, 3, 4, 5, 3, 1};
        int size1 = sizeof(arr1) / sizeof(arr1[0]);
        MountainArray* mountainArr1 = createMountainArray(arr1, size1);
        if (mountainArr1 != NULL) {
            int result1 = findInMountainArray(3, mountainArr1);
            printf("Test 1: %d (expected: 2)\\n", result1);
            freeMountainArray(mountainArr1);
        } else {
            printf("Test 1 failed: could not create array\\n");
        }
    }
    
    // Test case 2: target does not exist
    {
        int arr2[] = {0, 1, 2, 4, 2, 1};
        int size2 = sizeof(arr2) / sizeof(arr2[0]);
        MountainArray* mountainArr2 = createMountainArray(arr2, size2);
        if (mountainArr2 != NULL) {
            int result2 = findInMountainArray(3, mountainArr2);
            printf("Test 2: %d (expected: -1)\\n", result2);
            freeMountainArray(mountainArr2);
        } else {
            printf("Test 2 failed: could not create array\\n");
        }
    }
    
    // Test case 3: target at peak
    {
        int arr3[] = {1, 2, 3, 4, 5, 3, 1};
        int size3 = sizeof(arr3) / sizeof(arr3[0]);
        MountainArray* mountainArr3 = createMountainArray(arr3, size3);
        if (mountainArr3 != NULL) {
            int result3 = findInMountainArray(5, mountainArr3);
            printf("Test 3: %d (expected: 4)\\n", result3);
            freeMountainArray(mountainArr3);
        } else {
            printf("Test 3 failed: could not create array\\n");
        }
    }
    
    // Test case 4: target at beginning
    {
        int arr4[] = {1, 2, 3, 4, 5, 3, 1};
        int size4 = sizeof(arr4) / sizeof(arr4[0]);
        MountainArray* mountainArr4 = createMountainArray(arr4, size4);
        if (mountainArr4 != NULL) {
            int result4 = findInMountainArray(1, mountainArr4);
            printf("Test 4: %d (expected: 0)\\n", result4);
            freeMountainArray(mountainArr4);
        } else {
            printf("Test 4 failed: could not create array\\n");
        }
    }
    
    // Test case 5: small mountain array
    {
        int arr5[] = {1, 5, 2};
        int size5 = sizeof(arr5) / sizeof(arr5[0]);
        MountainArray* mountainArr5 = createMountainArray(arr5, size5);
        if (mountainArr5 != NULL) {
            int result5 = findInMountainArray(2, mountainArr5);
            printf("Test 5: %d (expected: 2)\\n", result5);
            freeMountainArray(mountainArr5);
        } else {
            printf("Test 5 failed: could not create array\\n");
        }
    }
    
    return 0;
}
