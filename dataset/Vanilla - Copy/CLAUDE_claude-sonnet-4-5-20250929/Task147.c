
#include <stdio.h>
#include <stdlib.h>

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyDown(int heap[], int size, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;
    
    if (left < size && heap[left] < heap[smallest])
        smallest = left;
    if (right < size && heap[right] < heap[smallest])
        smallest = right;
    
    if (smallest != idx) {
        swap(&heap[idx], &heap[smallest]);
        heapifyDown(heap, size, smallest);
    }
}

void heapifyUp(int heap[], int idx) {
    if (idx == 0) return;
    int parent = (idx - 1) / 2;
    if (heap[idx] < heap[parent]) {
        swap(&heap[idx], &heap[parent]);
        heapifyUp(heap, parent);
    }
}

int findKthLargest(int* nums, int numsSize, int k) {
    int* minHeap = (int*)malloc(k * sizeof(int));
    int heapSize = 0;
    
    for (int i = 0; i < numsSize; i++) {
        if (heapSize < k) {
            minHeap[heapSize] = nums[i];
            heapifyUp(minHeap, heapSize);
            heapSize++;
        } else if (nums[i] > minHeap[0]) {
            minHeap[0] = nums[i];
            heapifyDown(minHeap, heapSize, 0);
        }
    }
    
    int result = minHeap[0];
    free(minHeap);
    return result;
}

int main() {
    // Test case 1
    int nums1[] = {3, 2, 1, 5, 6, 4};
    printf("Test 1: %d\\n", findKthLargest(nums1, 6, 2)); // Expected: 5
    
    // Test case 2
    int nums2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    printf("Test 2: %d\\n", findKthLargest(nums2, 9, 4)); // Expected: 4
    
    // Test case 3
    int nums3[] = {7, 10, 4, 3, 20, 15};
    printf("Test 3: %d\\n", findKthLargest(nums3, 6, 3)); // Expected: 10
    
    // Test case 4
    int nums4[] = {1};
    printf("Test 4: %d\\n", findKthLargest(nums4, 1, 1)); // Expected: 1
    
    // Test case 5
    int nums5[] = {-1, -5, -3, -7, -2};
    printf("Test 5: %d\\n", findKthLargest(nums5, 5, 2)); // Expected: -2
    
    return 0;
}
