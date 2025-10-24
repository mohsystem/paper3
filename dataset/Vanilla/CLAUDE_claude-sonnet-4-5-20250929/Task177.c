
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
    dq->front = -1;
    dq->rear = 0;
    dq->capacity = capacity;
    return dq;
}

int isEmpty(Deque* dq) {
    return dq->front == -1;
}

void pushBack(Deque* dq, int val) {
    if (dq->front == -1) dq->front = 0;
    dq->data[dq->rear] = val;
    dq->rear = (dq->rear + 1) % dq->capacity;
}

void popFront(Deque* dq) {
    if (dq->front == -1) return;
    dq->front = (dq->front + 1) % dq->capacity;
    if (dq->front == dq->rear) dq->front = -1;
}

void popBack(Deque* dq) {
    if (dq->front == -1) return;
    dq->rear = (dq->rear - 1 + dq->capacity) % dq->capacity;
    if (dq->front == dq->rear) dq->front = -1;
}

int getFront(Deque* dq) {
    return dq->data[dq->front];
}

int getBack(Deque* dq) {
    return dq->data[(dq->rear - 1 + dq->capacity) % dq->capacity];
}

int max(int a, int b) {
    return a > b ? a : b;
}

int constrainedSubsetSum(int* nums, int numsSize, int k) {
    int* dp = (int*)malloc(numsSize * sizeof(int));
    Deque* dq = createDeque(numsSize);
    
    int maxSum = INT_MIN;
    
    for (int i = 0; i < numsSize; i++) {
        while (!isEmpty(dq) && getFront(dq) < i - k) {
            popFront(dq);
        }
        
        dp[i] = nums[i];
        if (!isEmpty(dq)) {
            dp[i] = max(dp[i], nums[i] + dp[getFront(dq)]);
        }
        
        while (!isEmpty(dq) && dp[getBack(dq)] <= dp[i]) {
            popBack(dq);
        }
        pushBack(dq, i);
        
        maxSum = max(maxSum, dp[i]);
    }
    
    free(dp);
    free(dq->data);
    free(dq);
    
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
    int nums5[] = {5, -3, 5};
    printf("Test 5: %d\\n", constrainedSubsetSum(nums5, 3, 1)); // Expected: 10
    
    return 0;
}
