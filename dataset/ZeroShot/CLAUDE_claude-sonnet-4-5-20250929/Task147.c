
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int *data;
    int size;
    int capacity;
} MinHeap;

MinHeap* createHeap(int capacity) {
    MinHeap *heap = (MinHeap*)malloc(sizeof(MinHeap));
    heap->data = (int*)malloc(capacity * sizeof(int));
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapifyUp(MinHeap *heap, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (heap->data[idx] < heap->data[parent]) {
            swap(&heap->data[idx], &heap->data[parent]);
            idx = parent;
        } else {
            break;
        }
    }
}

void heapifyDown(MinHeap *heap, int idx) {
    while (1) {
        int smallest = idx;
        int left = 2 * idx + 1;
        int right = 2 * idx + 2;
        
        if (left < heap->size && heap->data[left] < heap->data[smallest])
            smallest = left;
        if (right < heap->size && heap->data[right] < heap->data[smallest])
            smallest = right;
        
        if (smallest != idx) {
            swap(&heap->data[idx], &heap->data[smallest]);
            idx = smallest;
        } else {
            break;
        }
    }
}

void push(MinHeap *heap, int val) {
    if (heap->size < heap->capacity) {
        heap->data[heap->size] = val;
        heapifyUp(heap, heap->size);
        heap->size++;
    }
}

int pop(MinHeap *heap) {
    int result = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    heapifyDown(heap, 0);
    return result;
}

int findKthLargest(int *nums, int numsSize, int k) {
    if (nums == NULL || numsSize == 0 || k <= 0 || k > numsSize) {
        return -1;
    }
    
    MinHeap *heap = createHeap(k);
    
    for (int i = 0; i < numsSize; i++) {
        if (heap->size < k) {
            push(heap, nums[i]);
        } else if (nums[i] > heap->data[0]) {
            pop(heap);
            push(heap, nums[i]);
        }
    }
    
    int result = heap->data[0];
    free(heap->data);
    free(heap);
    return result;
}

int main() {
    // Test case 1
    int arr1[] = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    printf("Test 1: %d (Expected: 5)\\n", findKthLargest(arr1, 6, k1));
    
    // Test case 2
    int arr2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    printf("Test 2: %d (Expected: 4)\\n", findKthLargest(arr2, 9, k2));
    
    // Test case 3
    int arr3[] = {1};
    int k3 = 1;
    printf("Test 3: %d (Expected: 1)\\n", findKthLargest(arr3, 1, k3));
    
    // Test case 4
    int arr4[] = {7, 10, 4, 3, 20, 15};
    int k4 = 3;
    printf("Test 4: %d (Expected: 10)\\n", findKthLargest(arr4, 6, k4));
    
    // Test case 5
    int arr5[] = {-1, -5, -3, -2, -4};
    int k5 = 2;
    printf("Test 5: %d (Expected: -2)\\n", findKthLargest(arr5, 5, k5));
    
    return 0;
}
