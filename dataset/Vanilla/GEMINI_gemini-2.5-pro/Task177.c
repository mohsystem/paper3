#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

// --- Deque Implementation ---
typedef struct {
    int* arr;
    int front;
    int rear;
    int size;
    int capacity;
} Deque;

Deque* createDeque(int capacity) {
    Deque* dq = (Deque*)malloc(sizeof(Deque));
    dq->capacity = capacity;
    dq->front = 0;
    dq->rear = capacity - 1;
    dq->size = 0;
    dq->arr = (int*)malloc(dq->capacity * sizeof(int));
    return dq;
}

void freeDeque(Deque* dq) {
    free(dq->arr);
    free(dq);
}

int isEmpty(Deque* dq) {
    return dq->size == 0;
}

void push_back(Deque* dq, int value) {
    if (dq->size == dq->capacity) return;
    dq->rear = (dq->rear + 1) % dq->capacity;
    dq->arr[dq->rear] = value;
    dq->size++;
}

void pop_front(Deque* dq) {
    if (isEmpty(dq)) return;
    dq->front = (dq->front + 1) % dq->capacity;
    dq->size--;
}

void pop_back(Deque* dq) {
    if (isEmpty(dq)) return;
    dq->rear = (dq->rear - 1 + dq->capacity) % dq->capacity;
    dq->size--;
}

int get_front(Deque* dq) {
    return dq->arr[dq->front];
}

int get_back(Deque* dq) {
    return dq->arr[dq->rear];
}
// --- End of Deque Implementation ---

int max(int a, int b) {
    return a > b ? a : b;
}

int constrainedSubsetSum(int* nums, int numsSize, int k) {
    // dp[i] will store the maximum sum of a subsequence ending at index i
    int* dp = (int*)malloc(numsSize * sizeof(int));
    // A deque to store indices of dp elements, maintaining them in decreasing order of their values
    Deque* dq = createDeque(numsSize);
    int maxSum = INT_MIN;

    for (int i = 0; i < numsSize; ++i) {
        // Remove indices from the front of the deque that are out of the k-sized window
        while (!isEmpty(dq) && get_front(dq) < i - k) {
            pop_front(dq);
        }

        // The max sum of a subsequence ending in the previous window is at the front of the deque
        int prevMax = isEmpty(dq) ? 0 : dp[get_front(dq)];

        // Calculate dp[i]: it's nums[i] plus the max sum from the previous valid subsequence.
        // If the previous max sum is negative, we start a new subsequence from nums[i].
        dp[i] = nums[i] + max(0, prevMax);

        // Maintain the deque in decreasing order of dp values
        // Remove elements from the back of the deque that are smaller than the current dp[i]
        while (!isEmpty(dq) && dp[get_back(dq)] <= dp[i]) {
            pop_back(dq);
        }

        // Add the current index to the deque
        push_back(dq, i);

        // Update the overall maximum sum found so far
        maxSum = max(maxSum, dp[i]);
    }

    free(dp);
    freeDeque(dq);
    return maxSum;
}

int main() {
    // Test Case 1
    int nums1[] = {10, 2, -10, 5, 20};
    int k1 = 2;
    printf("Test Case 1: %d\n", constrainedSubsetSum(nums1, sizeof(nums1)/sizeof(nums1[0]), k1));

    // Test Case 2
    int nums2[] = {-1, -2, -3};
    int k2 = 1;
    printf("Test Case 2: %d\n", constrainedSubsetSum(nums2, sizeof(nums2)/sizeof(nums2[0]), k2));

    // Test Case 3
    int nums3[] = {10, -2, -10, -5, 20};
    int k3 = 2;
    printf("Test Case 3: %d\n", constrainedSubsetSum(nums3, sizeof(nums3)/sizeof(nums3[0]), k3));

    // Test Case 4
    int nums4[] = {-5, -4, -3, -2, -1};
    int k4 = 3;
    printf("Test Case 4: %d\n", constrainedSubsetSum(nums4, sizeof(nums4)/sizeof(nums4[0]), k4));

    // Test Case 5
    int nums5[] = {1, -5, 2, -6, 3, -7, 4};
    int k5 = 2;
    printf("Test Case 5: %d\n", constrainedSubsetSum(nums5, sizeof(nums5)/sizeof(nums5[0]), k5));

    return 0;
}