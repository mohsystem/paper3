
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
    dq->rear = 0;
    dq->capacity = capacity;
    return dq;
}

int isEmpty(Deque* dq) {
    return dq->front == dq->rear;
}

void pushBack(Deque* dq, int val) {
    dq->data[dq->rear++] = val;
}

void popBack(Deque* dq) {
    if (!isEmpty(dq)) {
        dq->rear--;
    }
}

void popFront(Deque* dq) {
    if (!isEmpty(dq)) {
        dq->front++;
    }
}

int getFront(Deque* dq) {
    return dq->data[dq->front];
}

int getBack(Deque* dq) {
    return dq->data[dq->rear - 1];
}

void freeDeque(Deque* dq) {
    free(dq->data);
    free(dq);
}

int shortestSubarray(int* nums, int numsSize, int k) {
    if (nums == NULL || numsSize == 0) {
        return -1;
    }
    
    long long* prefixSum = (long long*)calloc(numsSize + 1, sizeof(long long));
    
    // Calculate prefix sums
    for (int i = 0; i < numsSize; i++) {
        prefixSum[i + 1] = prefixSum[i] + nums[i];
    }
    
    int minLength = INT_MAX;
    Deque* dq = createDeque(numsSize + 1);
    
    for (int i = 0; i <= numsSize; i++) {
        // Remove indices from front where sum >= k
        while (!isEmpty(dq) && prefixSum[i] - prefixSum[getFront(dq)] >= k) {
            int len = i - getFront(dq);
            if (len < minLength) {
                minLength = len;
            }
            popFront(dq);
        }
        
        // Maintain monotonic increasing deque
        while (!isEmpty(dq) && prefixSum[i] <= prefixSum[getBack(dq)]) {
            popBack(dq);
        }
        
        pushBack(dq, i);
    }
    
    free(prefixSum);
    freeDeque(dq);
    
    return minLength == INT_MAX ? -1 : minLength;
}

int main() {
    // Test case 1
    int nums1[] = {1};
    printf("Test 1: %d\\n", shortestSubarray(nums1, 1, 1)); // Expected: 1
    
    // Test case 2
    int nums2[] = {1, 2};
    printf("Test 2: %d\\n", shortestSubarray(nums2, 2, 4)); // Expected: -1
    
    // Test case 3
    int nums3[] = {2, -1, 2};
    printf("Test 3: %d\\n", shortestSubarray(nums3, 3, 3)); // Expected: 3
    
    // Test case 4
    int nums4[] = {84, -37, 32, 40, 95};
    printf("Test 4: %d\\n", shortestSubarray(nums4, 5, 167)); // Expected: 3
    
    // Test case 5
    int nums5[] = {-28, 81, -20, 28, -29};
    printf("Test 5: %d\\n", shortestSubarray(nums5, 5, 89)); // Expected: 3
    
    return 0;
}
