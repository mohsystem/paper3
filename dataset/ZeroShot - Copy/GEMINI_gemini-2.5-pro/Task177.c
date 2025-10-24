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
    if (!dq) return NULL;
    dq->capacity = capacity;
    dq->arr = (int*)malloc(dq->capacity * sizeof(int));
    if (!dq->arr) {
        free(dq);
        return NULL;
    }
    dq->front = 0;
    dq->rear = 0;
    dq->size = 0;
    return dq;
}

int isEmpty(Deque* dq) { return dq->size == 0; }
int isFull(Deque* dq) { return dq->size == dq->capacity; }

void push_back(Deque* dq, int item) {
    if (isFull(dq)) return;
    dq->arr[dq->rear] = item;
    dq->rear = (dq->rear + 1) % dq->capacity;
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

int get_front(Deque* dq) { return dq->arr[dq->front]; }
int get_back(Deque* dq) { return dq->arr[(dq->rear - 1 + dq->capacity) % dq->capacity]; }

void freeDeque(Deque* dq) {
    if (dq != NULL) {
        free(dq->arr);
        free(dq);
    }
}
// --- End Deque Implementation ---

int max_int(int a, int b) {
    return a > b ? a : b;
}

/**
 * Finds the maximum sum of a non-empty subsequence of the array `nums`
 * such that for every two consecutive integers in the subsequence, nums[i] and nums[j],
 * where i < j, the condition j - i <= k is satisfied.
 *
 * @param nums The input integer array.
 * @param numsSize The size of the input array.
 * @param k The maximum distance between indices of consecutive elements in the subsequence.
 * @return The maximum subsequence sum.
 */
int constrainedSubsetSum(int* nums, int numsSize, int k) {
    if (numsSize <= 0) {
        return 0;
    }
    
    int* dp = (int*)malloc(numsSize * sizeof(int));
    if (dp == NULL) return INT_MIN;
    
    Deque* dq = createDeque(numsSize);
    if (dq == NULL) {
        free(dp);
        return INT_MIN;
    }
    
    int maxSum = INT_MIN;

    for (int i = 0; i < numsSize; ++i) {
        // 1. Remove indices from the front of the deque that are out of the window [i-k, i-1].
        if (!isEmpty(dq) && get_front(dq) < i - k) {
            pop_front(dq);
        }

        // 2. Calculate dp[i].
        int maxPrev = 0;
        if (!isEmpty(dq)) {
            maxPrev = dp[get_front(dq)];
        }
        dp[i] = nums[i] + max_int(0, maxPrev);

        // 3. Maintain the decreasing property of dp values in the deque.
        while (!isEmpty(dq) && dp[get_back(dq)] <= dp[i]) {
            pop_back(dq);
        }
        push_back(dq, i);

        // 4. Update the overall maximum sum.
        maxSum = max_int(maxSum, dp[i]);
    }

    free(dp);
    freeDeque(dq);
    return maxSum;
}

void run_test_case(const char* name, int* nums, int size, int k, int expected) {
    int result = constrainedSubsetSum(nums, size, k);
    printf("%s: %d\n", name, result);
}

int main() {
    // Test Case 1
    int nums1[] = {10, 2, -10, 5, 20};
    run_test_case("Test Case 1", nums1, sizeof(nums1)/sizeof(int), 2, 37);

    // Test Case 2
    int nums2[] = {-1, -2, -3};
    run_test_case("Test Case 2", nums2, sizeof(nums2)/sizeof(int), 1, -1);

    // Test Case 3
    int nums3[] = {10, -2, -10, -5, 20};
    run_test_case("Test Case 3", nums3, sizeof(nums3)/sizeof(int), 2, 23);

    // Test Case 4
    int nums4[] = {-5, -6};
    run_test_case("Test Case 4", nums4, sizeof(nums4)/sizeof(int), 2, -5);

    // Test Case 5
    int nums5[] = {1, -5, -6, 2, 8, -4, 3};
    run_test_case("Test Case 5", nums5, sizeof(nums5)/sizeof(int), 3, 14);

    return 0;
}