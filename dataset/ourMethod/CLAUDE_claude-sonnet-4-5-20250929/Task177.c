
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// Deque structure for monotonic queue optimization
// Using RAII pattern through proper allocation/deallocation (Rules#1)
typedef struct {
    int* data;      // Array to store indices
    int front;      // Front pointer
    int rear;       // Rear pointer
    int capacity;   // Maximum capacity
    int size;       // Current size
} Deque;

// Initialize deque with bounds checking (Rules#3)
Deque* createDeque(int capacity) {
    // Input validation (Rules#6)
    if (capacity <= 0 || capacity > 100000) {
        return NULL;
    }
    
    // Check malloc return value (Rules#1: C checklist)
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    if (dq == NULL) {
        return NULL;
    }
    
    // Initialize all fields (Rules#1: C checklist)
    dq->data = (int*)calloc(capacity, sizeof(int));
    if (dq->data == NULL) {
        free(dq);
        return NULL;
    }
    
    dq->front = 0;
    dq->rear = -1;
    dq->capacity = capacity;
    dq->size = 0;
    
    return dq;
}

// Free deque memory (Rules#1: C checklist - free exactly once)
void freeDeque(Deque* dq) {
    if (dq != NULL) {
        if (dq->data != NULL) {
            free(dq->data);
            dq->data = NULL;
        }
        free(dq);
    }
}

// Check if deque is empty
int isEmpty(Deque* dq) {
    return (dq == NULL || dq->size == 0);
}

// Get front element with bounds checking (Rules#3)
int getFront(Deque* dq) {
    if (isEmpty(dq)) {
        return -1;
    }
    return dq->data[dq->front];
}

// Get back element with bounds checking (Rules#3)
int getBack(Deque* dq) {
    if (isEmpty(dq)) {
        return -1;
    }
    return dq->data[dq->rear];
}

// Push back with bounds checking (Rules#3, Rules#4)
void pushBack(Deque* dq, int value) {
    if (dq == NULL || dq->size >= dq->capacity) {
        return;
    }
    dq->rear = (dq->rear + 1) % dq->capacity;
    dq->data[dq->rear] = value;
    dq->size++;
}

// Pop front with bounds checking (Rules#3)
void popFront(Deque* dq) {
    if (isEmpty(dq)) {
        return;
    }
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
}

// Pop back with bounds checking (Rules#3)
void popBack(Deque* dq) {
    if (isEmpty(dq)) {
        return;
    }
    dq->rear = (dq->rear - 1 + dq->capacity) % dq->capacity;
    dq->size--;
}

// Main algorithm with complete input validation (Rules#1, Rules#6)
int constrainedSubsetSum(int* nums, int numsSize, int k) {
    // Input validation (Rules#6)
    if (nums == NULL || numsSize <= 0 || k < 1) {
        return INT_MIN;
    }
    
    // Validate k range (Rules#6)
    if (k > numsSize) {
        k = numsSize;
    }
    
    // Allocate dp array with NULL check (Rules#1: C checklist)
    int* dp = (int*)calloc(numsSize, sizeof(int));
    if (dp == NULL) {
        return INT_MIN;
    }
    
    // Create deque with error checking (Rules#1)
    Deque* dq = createDeque(numsSize);
    if (dq == NULL) {
        free(dp);
        return INT_MIN;
    }
    
    int maxSum = INT_MIN;
    
    // Process each element with bounds checking (Rules#3)
    for (int i = 0; i < numsSize; i++) {
        // Remove indices outside k-window with bounds check (Rules#3)
        while (!isEmpty(dq) && getFront(dq) < i - k) {
            popFront(dq);
        }
        
        // Calculate dp[i] with bounds checking (Rules#3)
        if (isEmpty(dq)) {
            dp[i] = nums[i];
        } else {
            int frontIdx = getFront(dq);
            // Bounds check before array access (Rules#3)
            if (frontIdx >= 0 && frontIdx < numsSize) {
                int maxPrev = dp[frontIdx];
                dp[i] = (nums[i] > nums[i] + maxPrev) ? nums[i] : nums[i] + maxPrev;
            } else {
                dp[i] = nums[i];
            }
        }
        
        // Maintain monotonic decreasing deque with bounds checks (Rules#3)
        while (!isEmpty(dq)) {
            int backIdx = getBack(dq);
            if (backIdx >= 0 && backIdx < numsSize && dp[backIdx] <= dp[i]) {
                popBack(dq);
            } else {
                break;
            }
        }
        
        pushBack(dq, i);
        
        // Update maximum
        if (dp[i] > maxSum) {
            maxSum = dp[i];
        }
    }
    
    // Clean up memory (Rules#1: C checklist - free exactly once)
    freeDeque(dq);
    free(dp);
    
    return maxSum;
}

int main() {
    // Test case 1
    int nums1[] = {10, 2, -10, 5, 20};
    int result1 = constrainedSubsetSum(nums1, 5, 2);
    printf("Test 1: %d (expected: 37)\\n", result1);
    
    // Test case 2
    int nums2[] = {-1, -2, -3};
    int result2 = constrainedSubsetSum(nums2, 3, 1);
    printf("Test 2: %d (expected: -1)\\n", result2);
    
    // Test case 3
    int nums3[] = {10, -2, -10, -5, 20};
    int result3 = constrainedSubsetSum(nums3, 5, 2);
    printf("Test 3: %d (expected: 23)\\n", result3);
    
    // Test case 4: Single element
    int nums4[] = {5};
    int result4 = constrainedSubsetSum(nums4, 1, 1);
    printf("Test 4: %d (expected: 5)\\n", result4);
    
    // Test case 5: All positive
    int nums5[] = {1, 2, 3, 4, 5};
    int result5 = constrainedSubsetSum(nums5, 5, 2);
    printf("Test 5: %d (expected: 15)\\n", result5);
    
    return 0;
}
