
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

// Deque structure for storing indices
typedef struct {
    int* data;      // Array to store indices
    int front;      // Front index
    int rear;       // Rear index
    int capacity;   // Maximum capacity
    int size;       // Current size
} Deque;

// Initialize deque with given capacity
Deque* dequeCreate(int capacity) {
    // Input validation: capacity must be positive
    if (capacity <= 0 || capacity > 100000) {
        return NULL;
    }
    
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    if (dq == NULL) {
        return NULL; // Memory allocation failed
    }
    
    dq->data = (int*)malloc(sizeof(int) * capacity);
    if (dq->data == NULL) {
        free(dq);
        return NULL; // Memory allocation failed
    }
    
    dq->front = 0;
    dq->rear = -1;
    dq->capacity = capacity;
    dq->size = 0;
    
    return dq;
}

// Free deque memory
void dequeFree(Deque* dq) {
    if (dq != NULL) {
        if (dq->data != NULL) {
            free(dq->data);
            dq->data = NULL;
        }
        free(dq);
    }
}

// Check if deque is empty
bool dequeIsEmpty(Deque* dq) {
    return dq == NULL || dq->size == 0;
}

// Get front element
int dequeFront(Deque* dq) {
    if (dequeIsEmpty(dq)) {
        return -1; // Error indicator
    }
    return dq->data[dq->front];
}

// Get back element
int dequeBack(Deque* dq) {
    if (dequeIsEmpty(dq)) {
        return -1; // Error indicator
    }
    return dq->data[dq->rear];
}

// Push to back
bool dequePushBack(Deque* dq, int value) {
    if (dq == NULL || dq->size >= dq->capacity) {
        return false; // Deque full or NULL
    }
    dq->rear = (dq->rear + 1) % dq->capacity;
    dq->data[dq->rear] = value;
    dq->size++;
    return true;
}

// Pop from front
bool dequePopFront(Deque* dq) {
    if (dequeIsEmpty(dq)) {
        return false;
    }
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
    return true;
}

// Pop from back
bool dequePopBack(Deque* dq) {
    if (dequeIsEmpty(dq)) {
        return false;
    }
    dq->rear = (dq->rear - 1 + dq->capacity) % dq->capacity;
    dq->size--;
    return true;
}

// Main function to find maximum in sliding window
int* maxSlidingWindow(const int* nums, int numsSize, int k, int* returnSize) {
    // Initialize return size to 0 for error cases
    if (returnSize == NULL) {
        return NULL;
    }
    *returnSize = 0;
    
    // Input validation: check for NULL pointer
    if (nums == NULL) {
        return NULL;
    }
    
    // Input validation: check array size bounds
    if (numsSize <= 0 || numsSize > 100000) {
        return NULL;
    }
    
    // Input validation: check k bounds
    if (k < 1 || k > numsSize) {
        return NULL;
    }
    
    // Calculate result size with overflow check
    int resultSize = numsSize - k + 1;
    if (resultSize <= 0 || resultSize > 100000) {
        return NULL;
    }
    
    // Allocate result array
    int* result = (int*)malloc(sizeof(int) * resultSize);
    if (result == NULL) {
        return NULL; // Memory allocation failed
    }
    
    // Initialize result array to prevent uninitialized memory
    memset(result, 0, sizeof(int) * resultSize);
    
    // Create deque
    Deque* dq = dequeCreate(numsSize);
    if (dq == NULL) {
        free(result);
        return NULL;
    }
    
    int resultIndex = 0;
    
    for (int i = 0; i < numsSize; i++) {
        // Remove indices outside current window
        while (!dequeIsEmpty(dq) && dequeFront(dq) <= i - k) {
            dequePopFront(dq);
        }
        
        // Remove indices whose values are smaller than current value
        while (!dequeIsEmpty(dq)) {
            int backIdx = dequeBack(dq);
            // Bounds check for array access
            if (backIdx < 0 || backIdx >= numsSize) {
                break;
            }
            if (nums[backIdx] <= nums[i]) {
                dequePopBack(dq);
            } else {
                break;
            }
        }
        
        dequePushBack(dq, i);
        
        // Record result when window is complete
        if (i >= k - 1) {
            int frontIdx = dequeFront(dq);
            // Bounds check before array access
            if (frontIdx >= 0 && frontIdx < numsSize && resultIndex < resultSize) {
                result[resultIndex++] = nums[frontIdx];
            }
        }
    }
    
    dequeFree(dq);
    *returnSize = resultIndex;
    return result;
}

int main() {
    // Test case 1
    int test1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int returnSize1 = 0;
    int* result1 = maxSlidingWindow(test1, 8, 3, &returnSize1);
    printf("Test 1: ");
    if (result1 != NULL) {
        for (int i = 0; i < returnSize1; i++) {
            printf("%d ", result1[i]);
        }
        free(result1);
    }
    printf("\\n");
    
    // Test case 2
    int test2[] = {1};
    int returnSize2 = 0;
    int* result2 = maxSlidingWindow(test2, 1, 1, &returnSize2);
    printf("Test 2: ");
    if (result2 != NULL) {
        for (int i = 0; i < returnSize2; i++) {
            printf("%d ", result2[i]);
        }
        free(result2);
    }
    printf("\\n");
    
    // Test case 3
    int test3[] = {1, -1, 5, 3};
    int returnSize3 = 0;
    int* result3 = maxSlidingWindow(test3, 4, 4, &returnSize3);
    printf("Test 3: ");
    if (result3 != NULL) {
        for (int i = 0; i < returnSize3; i++) {
            printf("%d ", result3[i]);
        }
        free(result3);
    }
    printf("\\n");
    
    // Test case 4
    int test4[] = {-7, -8, -3, -1, -4};
    int returnSize4 = 0;
    int* result4 = maxSlidingWindow(test4, 5, 2, &returnSize4);
    printf("Test 4: ");
    if (result4 != NULL) {
        for (int i = 0; i < returnSize4; i++) {
            printf("%d ", result4[i]);
        }
        free(result4);
    }
    printf("\\n");
    
    // Test case 5
    int test5[] = {9, 7, 5, 3, 1};
    int returnSize5 = 0;
    int* result5 = maxSlidingWindow(test5, 5, 3, &returnSize5);
    printf("Test 5: ");
    if (result5 != NULL) {
        for (int i = 0; i < returnSize5; i++) {
            printf("%d ", result5[i]);
        }
        free(result5);
    }
    printf("\\n");
    
    return 0;
}
