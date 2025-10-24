
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

typedef struct {
    int* data;
    int front;
    int rear;
    int capacity;
} Deque;

Deque* createDeque(int capacity) {
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    dq->data = (int*)malloc(sizeof(int) * capacity);
    dq->front = 0;
    dq->rear = 0;
    dq->capacity = capacity;
    return dq;
}

void freeDeque(Deque* dq) {
    free(dq->data);
    free(dq);
}

int isEmpty(Deque* dq) {
    return dq->front == dq->rear;
}

void pushBack(Deque* dq, int val) {
    dq->data[dq->rear++] = val;
}

void popBack(Deque* dq) {
    if (!isEmpty(dq)) dq->rear--;
}

void popFront(Deque* dq) {
    if (!isEmpty(dq)) dq->front++;
}

int front(Deque* dq) {
    return dq->data[dq->front];
}

int back(Deque* dq) {
    return dq->data[dq->rear - 1];
}

int constrainedSubsetSum(int* nums, int numsSize, int k) {
    if (nums == NULL || numsSize == 0) {
        return INT_MIN;
    }
    if (k <= 0 || k > numsSize) {
        return INT_MIN;
    }
    
    int* dp = (int*)malloc(sizeof(int) * numsSize);
    Deque* dq = createDeque(numsSize);
    int maxSum = INT_MIN;
    
    for (int i = 0; i < numsSize; i++) {
        // Remove elements outside the window
        while (!isEmpty(dq) && front(dq) < i - k) {
            popFront(dq);
        }
        
        // Calculate dp[i]
        dp[i] = nums[i];
        if (!isEmpty(dq)) {
            int maxPrev = dp[front(dq)];
            if (nums[i] + maxPrev > dp[i]) {
                dp[i] = nums[i] + maxPrev;
            }
        }
        
        // Maintain deque in decreasing order of dp values
        while (!isEmpty(dq) && dp[back(dq)] <= dp[i]) {
            popBack(dq);
        }
        pushBack(dq, i);
        
        if (dp[i] > maxSum) {
            maxSum = dp[i];
        }
    }
    
    free(dp);
    freeDeque(dq);
    return maxSum;
}

int main() {
    // Test case 1
    int nums1[] = {10, 2, -10, 5, 20};
    printf("Test 1: %d\\n", constrainedSubsetSum(nums1, 5, 2)); // Expected: 37
    
    // Test case 2
    int nums2[] = {-1, -2, -3};
    printf("Test 2: %d\\n", constrainedSubsetSum(nums2, 3, 1)); // Expected: -1
    
    // Test case 3
    int nums3[] = {10, -2, -10, -5, 20};
    printf("Test 3: %d\\n", constrainedSubsetSum(nums3, 5, 2)); // Expected: 23
    
    // Test case 4
    int nums4[] = {5, -3, 5};
    printf("Test 4: %d\\n", constrainedSubsetSum(nums4, 3, 2)); // Expected: 10
    
    // Test case 5
    int nums5[] = {1, 2, 3, 4, 5};
    printf("Test 5: %d\\n", constrainedSubsetSum(nums5, 5, 3)); // Expected: 15
    
    return 0;
}
