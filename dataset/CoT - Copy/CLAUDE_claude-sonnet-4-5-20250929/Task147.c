
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Min heap structure
typedef struct {
    int* data;
    int size;
    int capacity;
} MinHeap;

// Create a new min heap
MinHeap* createMinHeap(int capacity) {
    MinHeap* heap = (MinHeap*)malloc(sizeof(MinHeap));
    if (!heap) return NULL;
    
    heap->data = (int*)malloc(sizeof(int) * capacity);
    if (!heap->data) {
        free(heap);
        return NULL;
    }
    
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// Swap two integers
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Heapify up
void heapifyUp(MinHeap* heap, int index) {
    if (index == 0) return;
    
    int parent = (index - 1) / 2;
    if (heap->data[index] < heap->data[parent]) {
        swap(&heap->data[index], &heap->data[parent]);
        heapifyUp(heap, parent);
    }
}

// Heapify down
void heapifyDown(MinHeap* heap, int index) {
    int smallest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    
    if (left < heap->size && heap->data[left] < heap->data[smallest]) {
        smallest = left;
    }
    if (right < heap->size && heap->data[right] < heap->data[smallest]) {
        smallest = right;
    }
    
    if (smallest != index) {
        swap(&heap->data[index], &heap->data[smallest]);
        heapifyDown(heap, smallest);
    }
}

// Insert into min heap
void insert(MinHeap* heap, int value) {
    if (heap->size >= heap->capacity) return;
    
    heap->data[heap->size] = value;
    heapifyUp(heap, heap->size);
    heap->size++;
}

// Extract minimum
int extractMin(MinHeap* heap) {
    if (heap->size == 0) return -1;
    
    int min = heap->data[0];
    heap->data[0] = heap->data[heap->size - 1];
    heap->size--;
    heapifyDown(heap, 0);
    
    return min;
}

// Get minimum
int getMin(MinHeap* heap) {
    if (heap->size == 0) return -1;
    return heap->data[0];
}

// Find kth largest element
int findKthLargest(int* nums, int numsSize, int k) {
    // Input validation
    if (nums == NULL || numsSize == 0) {
        fprintf(stderr, "Error: Array cannot be NULL or empty\\n");
        return -1;
    }
    if (k <= 0 || k > numsSize) {
        fprintf(stderr, "Error: k must be between 1 and array length\\n");
        return -1;
    }
    
    MinHeap* heap = createMinHeap(k);
    if (!heap) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return -1;
    }
    
    for (int i = 0; i < numsSize; i++) {
        insert(heap, nums[i]);
        if (heap->size > k) {
            extractMin(heap);
        }
    }
    
    int result = getMin(heap);
    
    // Clean up
    free(heap->data);
    free(heap);
    
    return result;
}

int main() {
    // Test case 1: Normal case
    int test1[] = {3, 2, 1, 5, 6, 4};
    int k1 = 2;
    printf("Test 1: Array: [3, 2, 1, 5, 6, 4], k=%d\\n", k1);
    printf("Result: %d\\n\\n", findKthLargest(test1, 6, k1));
    
    // Test case 2: Array with duplicates
    int test2[] = {3, 2, 3, 1, 2, 4, 5, 5, 6};
    int k2 = 4;
    printf("Test 2: Array: [3, 2, 3, 1, 2, 4, 5, 5, 6], k=%d\\n", k2);
    printf("Result: %d\\n\\n", findKthLargest(test2, 9, k2));
    
    // Test case 3: k = 1 (largest element)
    int test3[] = {7, 10, 4, 3, 20, 15};
    int k3 = 1;
    printf("Test 3: Array: [7, 10, 4, 3, 20, 15], k=%d\\n", k3);
    printf("Result: %d\\n\\n", findKthLargest(test3, 6, k3));
    
    // Test case 4: k equals array length (smallest element)
    int test4[] = {5, 2, 8, 1, 9};
    int k4 = 5;
    printf("Test 4: Array: [5, 2, 8, 1, 9], k=%d\\n", k4);
    printf("Result: %d\\n\\n", findKthLargest(test4, 5, k4));
    
    // Test case 5: Single element array
    int test5[] = {42};
    int k5 = 1;
    printf("Test 5: Array: [42], k=%d\\n", k5);
    printf("Result: %d\\n", findKthLargest(test5, 1, k5));
    
    return 0;
}
