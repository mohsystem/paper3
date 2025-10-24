
#include <stdio.h>
#include <stdlib.h>

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

void popFront(Deque* dq) {
    if (!isEmpty(dq)) dq->front++;
}

void popBack(Deque* dq) {
    if (!isEmpty(dq)) dq->rear--;
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

int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    // Input validation
    if (nums == NULL || numsSize == 0 || k <= 0 || k > numsSize) {
        *returnSize = 0;
        return NULL;
    }
    
    *returnSize = numsSize - k + 1;
    int* result = (int*)malloc((*returnSize) * sizeof(int));
    Deque* dq = createDeque(numsSize);
    
    for (int i = 0; i < numsSize; i++) {
        // Remove indices that are out of current window
        while (!isEmpty(dq) && getFront(dq) < i - k + 1) {
            popFront(dq);
        }
        
        // Remove indices whose corresponding values are less than current element
        while (!isEmpty(dq) && nums[getBack(dq)] < nums[i]) {
            popBack(dq);
        }
        
        // Add current index
        pushBack(dq, i);
        
        // Add to result when window is complete
        if (i >= k - 1) {
            result[i - k + 1] = nums[getFront(dq)];
        }
    }
    
    freeDeque(dq);
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {1, 3, -1, -3, 5, 3, 6, 7};
    int k1 = 3;
    int returnSize1;
    int* res1 = maxSlidingWindow(nums1, 8, k1, &returnSize1);
    printf("Test 1: ");
    for (int i = 0; i < returnSize1; i++) printf("%d ", res1[i]);
    printf("\\n");
    free(res1);
    
    // Test case 2
    int nums2[] = {1};
    int k2 = 1;
    int returnSize2;
    int* res2 = maxSlidingWindow(nums2, 1, k2, &returnSize2);
    printf("Test 2: ");
    for (int i = 0; i < returnSize2; i++) printf("%d ", res2[i]);
    printf("\\n");
    free(res2);
    
    // Test case 3
    int nums3[] = {1, -1};
    int k3 = 1;
    int returnSize3;
    int* res3 = maxSlidingWindow(nums3, 2, k3, &returnSize3);
    printf("Test 3: ");
    for (int i = 0; i < returnSize3; i++) printf("%d ", res3[i]);
    printf("\\n");
    free(res3);
    
    // Test case 4
    int nums4[] = {9, 11};
    int k4 = 2;
    int returnSize4;
    int* res4 = maxSlidingWindow(nums4, 2, k4, &returnSize4);
    printf("Test 4: ");
    for (int i = 0; i < returnSize4; i++) printf("%d ", res4[i]);
    printf("\\n");
    free(res4);
    
    // Test case 5
    int nums5[] = {4, -2};
    int k5 = 2;
    int returnSize5;
    int* res5 = maxSlidingWindow(nums5, 2, k5, &returnSize5);
    printf("Test 5: ");
    for (int i = 0; i < returnSize5; i++) printf("%d ", res5[i]);
    printf("\\n");
    free(res5);
    
    return 0;
}
