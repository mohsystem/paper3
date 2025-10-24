
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
    dq->data = (int*)malloc(capacity * sizeof(int));
    dq->front = 0;
    dq->rear = -1;
    dq->capacity = capacity;
    return dq;
}

int isEmpty(Deque* dq) {
    return dq->rear < dq->front;
}

void pushBack(Deque* dq, int val) {
    dq->data[++dq->rear] = val;
}

void popFront(Deque* dq) {
    if (!isEmpty(dq)) dq->front++;
}

void popBack(Deque* dq) {
    if (!isEmpty(dq)) dq->rear--;
}

int front(Deque* dq) {
    return dq->data[dq->front];
}

int back(Deque* dq) {
    return dq->data[dq->rear];
}

void freeDeque(Deque* dq) {
    free(dq->data);
    free(dq);
}

int max(int a, int b) {
    return a > b ? a : b;
}

int constrainedSubsetSum(int* nums, int numsSize, int k) {
    int* dp = (int*)malloc(numsSize * sizeof(int));
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
            dp[i] = max(dp[i], nums[i] + dp[front(dq)]);
        }
        
        // Maintain deque in decreasing order of dp values
        while (!isEmpty(dq) && dp[back(dq)] <= dp[i]) {
            popBack(dq);
        }
        
        pushBack(dq, i);
        maxSum = max(maxSum, dp[i]);
    }
    
    freeDeque(dq);
    free(dp);
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
    int nums4[] = {1, 2, 3, 4, 5};
    printf("Test 4: %d\\n", constrainedSubsetSum(nums4, 5, 3)); // Expected: 15
    
    // Test case 5
    int nums5[] = {-5, -3, -1};
    printf("Test 5: %d\\n", constrainedSubsetSum(nums5, 3, 2)); // Expected: -1
    
    return 0;
}
