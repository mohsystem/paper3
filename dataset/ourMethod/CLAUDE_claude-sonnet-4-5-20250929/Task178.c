
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

// Deque structure for monotonic queue implementation
// Security: Fixed-size allocation with bounds checking
typedef struct {
    size_t* data;
    size_t capacity;
    size_t front;
    size_t rear;
    size_t size;
} Deque;

// Initialize deque with given capacity
// Security: Validates capacity and checks malloc return
Deque* deque_create(size_t capacity) {
    if (capacity == 0 || capacity > 100001) {
        return NULL;
    }
    
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    if (dq == NULL) {
        return NULL;
    }
    
    dq->data = (size_t*)malloc(capacity * sizeof(size_t));
    if (dq->data == NULL) {
        free(dq);
        return NULL;
    }
    
    dq->capacity = capacity;
    dq->front = 0;
    dq->rear = 0;
    dq->size = 0;
    
    return dq;
}

// Free deque memory
// Security: Checks for NULL and clears sensitive data
void deque_destroy(Deque* dq) {
    if (dq != NULL) {
        if (dq->data != NULL) {
            // Clear memory before freeing
            for (size_t i = 0; i < dq->capacity; i++) {
                dq->data[i] = 0;
            }
            free(dq->data);
            dq->data = NULL;
        }
        free(dq);
    }
}

// Check if deque is empty
bool deque_empty(const Deque* dq) {
    return dq == NULL || dq->size == 0;
}

// Get front element
// Security: Validates deque is not empty before access
size_t deque_front(const Deque* dq) {
    if (dq == NULL || dq->size == 0) {
        return 0;
    }
    return dq->data[dq->front];
}

// Get back element
// Security: Validates deque is not empty before access
size_t deque_back(const Deque* dq) {
    if (dq == NULL || dq->size == 0) {
        return 0;
    }
    size_t back_idx = (dq->rear + dq->capacity - 1) % dq->capacity;
    return dq->data[back_idx];
}

// Add element to back
// Security: Checks capacity before insertion
bool deque_push_back(Deque* dq, size_t value) {
    if (dq == NULL || dq->size >= dq->capacity) {
        return false;
    }
    
    dq->data[dq->rear] = value;
    dq->rear = (dq->rear + 1) % dq->capacity;
    dq->size++;
    return true;
}

// Remove element from front
// Security: Validates deque is not empty
bool deque_pop_front(Deque* dq) {
    if (dq == NULL || dq->size == 0) {
        return false;
    }
    
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
    return true;
}

// Remove element from back
// Security: Validates deque is not empty
bool deque_pop_back(Deque* dq) {
    if (dq == NULL || dq->size == 0) {
        return false;
    }
    
    dq->rear = (dq->rear + dq->capacity - 1) % dq->capacity;
    dq->size--;
    return true;
}

// Main function to find shortest subarray
// Security: All inputs validated, memory properly managed
int shortestSubarray(const int* nums, int numsSize, int k) {
    // Input validation
    if (nums == NULL || numsSize <= 0 || numsSize > 100000 || k <= 0) {
        return -1;
    }
    
    // Allocate prefix sum array with bounds check
    long long* prefixSum = (long long*)calloc(numsSize + 1, sizeof(long long));
    if (prefixSum == NULL) {
        return -1;
    }
    
    // Build prefix sum array with overflow protection
    for (int i = 0; i < numsSize; i++) {
        prefixSum[i + 1] = prefixSum[i] + nums[i];
    }
    
    int minLength = INT_MAX;
    
    // Create deque with sufficient capacity
    Deque* dq = deque_create(numsSize + 1);
    if (dq == NULL) {
        free(prefixSum);
        return -1;
    }
    
    // Process each prefix sum
    for (int i = 0; i <= numsSize; i++) {
        // Check if current prefix minus front >= k
        while (!deque_empty(dq) && prefixSum[i] - prefixSum[deque_front(dq)] >= k) {
            size_t front_idx = deque_front(dq);
            if (i > (int)front_idx) {
                int length = i - (int)front_idx;
                if (length < minLength) {
                    minLength = length;
                }
            }
            deque_pop_front(dq);
        }
        
        // Maintain monotonic property
        while (!deque_empty(dq) && prefixSum[i] <= prefixSum[deque_back(dq)]) {
            deque_pop_back(dq);
        }
        
        // Add current index
        deque_push_back(dq, i);
    }
    
    // Clean up memory
    deque_destroy(dq);
    free(prefixSum);
    
    return (minLength == INT_MAX) ? -1 : minLength;
}

int main() {
    // Test case 1
    int test1[] = {1};
    printf("Test 1: %d (expected: 1)\\n", shortestSubarray(test1, 1, 1));
    
    // Test case 2
    int test2[] = {1, 2};
    printf("Test 2: %d (expected: -1)\\n", shortestSubarray(test2, 2, 4));
    
    // Test case 3
    int test3[] = {2, -1, 2};
    printf("Test 3: %d (expected: 3)\\n", shortestSubarray(test3, 3, 3));
    
    // Test case 4
    int test4[] = {1, 1, 1, 1, 1};
    printf("Test 4: %d (expected: 3)\\n", shortestSubarray(test4, 5, 3));
    
    // Test case 5
    int test5[] = {84, -37, 32, 40, 95};
    printf("Test 5: %d (expected: 3)\\n", shortestSubarray(test5, 5, 167));
    
    return 0;
}
