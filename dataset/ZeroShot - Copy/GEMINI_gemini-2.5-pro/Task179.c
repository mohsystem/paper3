#include <stdio.h>
#include <stdlib.h>

// --- Deque Implementation (Circular Array) for storing indices ---
typedef struct {
    int* data;
    int capacity;
    int front;
    int size;
} Deque;

Deque* create_deque(int capacity) {
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    if (!dq) return NULL;
    dq->capacity = capacity > 0 ? capacity : 1;
    dq->data = (int*)malloc(dq->capacity * sizeof(int));
    if (!dq->data) {
        free(dq);
        return NULL;
    }
    dq->front = 0;
    dq->size = 0;
    return dq;
}

void destroy_deque(Deque* dq) {
    if (dq) {
        free(dq->data);
        free(dq);
    }
}

int is_deque_empty(const Deque* dq) {
    return dq->size == 0;
}

void dq_push_back(Deque* dq, int value) {
    if (dq->size == dq->capacity) return; // Should not happen if capacity is k
    int rear_index = (dq->front + dq->size) % dq->capacity;
    dq->data[rear_index] = value;
    dq->size++;
}

void dq_pop_back(Deque* dq) {
    if (is_deque_empty(dq)) return;
    dq->size--;
}

void dq_pop_front(Deque* dq) {
    if (is_deque_empty(dq)) return;
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
}

int dq_front_val(const Deque* dq) {
    return dq->data[dq->front];
}

int dq_back_val(const Deque* dq) {
    int rear_index = (dq->front + dq->size - 1 + dq->capacity) % dq->capacity;
    return dq->data[rear_index];
}
// --- End Deque Implementation ---


/**
 * Note: The returned array must be malloced, assume caller calls free().
 * The size of the returned array will be stored in *returnSize.
 */
int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    // Handle edge cases based on constraints
    if (nums == NULL || numsSize == 0 || k <= 0) {
        *returnSize = 0;
        return NULL;
    }

    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc(*returnSize * sizeof(int));
    if (!result) {
        *returnSize = 0;
        return NULL;
    }
    int resultIndex = 0;

    // The deque can hold at most k indices
    Deque* dq = create_deque(k);
    if (!dq) {
        free(result);
        *returnSize = 0;
        return NULL;
    }

    for (int i = 0; i < numsSize; i++) {
        // 1. Remove indices from the front that are out of the window
        if (!is_deque_empty(dq) && dq_front_val(dq) <= i - k) {
            dq_pop_front(dq);
        }
        // 2. Maintain decreasing order of values in the deque
        while (!is_deque_empty(dq) && nums[dq_back_val(dq)] < nums[i]) {
            dq_pop_back(dq);
        }
        // 3. Add current index to the back
        dq_push_back(dq, i);
        // 4. If window is formed, store max
        if (i >= k - 1) {
            result[resultIndex++] = nums[dq_front_val(dq)];
        }
    }

    destroy_deque(dq);
    return result;
}

void print_array(const char* prefix, int* arr, int size) {
    printf("%s[", prefix);
    for (int i = 0; i < size; ++i) {
        printf("%d%s", arr[i], (i == size - 1) ? "" : ", ");
    }
    printf("]");
}

void run_test_case(const char* name, int* nums, int numsSize, int k, int* expected, int expectedSize) {
    printf("%s:\n", name);
    print_array("Input: nums = ", nums, numsSize);
    printf(", k = %d\n", k);
    
    int returnSize;
    int* result = maxSlidingWindow(nums, numsSize, k, &returnSize);
    
    print_array("Output: ", result, returnSize);
    printf("\n");
    print_array("Expected: ", expected, expectedSize);
    printf("\n\n");
    
    free(result); // Free the memory allocated by the function
}

int main() {
    // Test Case 1
    int nums1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int expected1[] = {3, 3, 5, 5, 6, 7};
    run_test_case("Test Case 1", nums1, sizeof(nums1)/sizeof(int), 3, expected1, sizeof(expected1)/sizeof(int));

    // Test Case 2
    int nums2[] = {1};
    int expected2[] = {1};
    run_test_case("Test Case 2", nums2, sizeof(nums2)/sizeof(int), 1, expected2, sizeof(expected2)/sizeof(int));

    // Test Case 3
    int nums3[] = {1, -1};
    int expected3[] = {1, -1};
    run_test_case("Test Case 3", nums3, sizeof(nums3)/sizeof(int), 1, expected3, sizeof(expected3)/sizeof(int));

    // Test Case 4
    int nums4[] = {9, 10, 9, -7, -4, -8, 2, -6};
    int expected4[] = {10, 10, 9, 2, 2};
    run_test_case("Test Case 4", nums4, sizeof(nums4)/sizeof(int), 5, expected4, sizeof(expected4)/sizeof(int));

    // Test Case 5
    int nums5[] = {7, 2, 4};
    int expected5[] = {7, 4};
    run_test_case("Test Case 5", nums5, sizeof(nums5)/sizeof(int), 2, expected5, sizeof(expected5)/sizeof(int));

    return 0;
}